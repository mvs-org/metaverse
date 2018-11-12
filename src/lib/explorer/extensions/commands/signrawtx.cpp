/**
 * Copyright (c) 2016-2018 mvs developers
 *
 * This file is part of metaverse-explorer.
 *
 * metaverse-explorer is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <metaverse/explorer/json_helper.hpp>
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/signrawtx.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using payment_address = wallet::payment_address;

console_result signrawtx::invoke(Json::Value& jv_output,
                                 libbitcoin::server::server_node& node) {
    auto &blockchain = node.chain_impl();
    const auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    tx_type tx_ = argument_.transaction;

    std::map<uint32_t, std::map<std::string, std::string> > script_sig_map;
    // sign tx
    {
        uint32_t index = 0;
        chain::transaction tx_temp;
        uint64_t tx_height;

        for (auto &fromeach : tx_.inputs) {
            if (!(blockchain.get_transaction(fromeach.previous_output.hash, tx_temp, tx_height)))
                throw argument_legality_exception{
                        "invalid transaction hash " + encode_hash(fromeach.previous_output.hash)};

            auto output = tx_temp.outputs.at(fromeach.previous_output.index);

            // get address private key
            auto address = payment_address::extract(output.script);
            // script address : maybe multisig
            if (!address || (address.version() == 0x5)) {
                const auto scirpt_vec = acc->get_script(address.encoded());
                if (!scirpt_vec || scirpt_vec->empty()) {
                    throw argument_legality_exception{"invalid script: " + config::script(output.script).to_string()};
                }
                const auto &scirpt = scirpt_vec->begin();

                // watch-only address
                const data_chunk &bin_script = scirpt->get_script();
                if (bin_script.empty()) {
                    throw argument_legality_exception{"watch-only script address: " + config::script(output.script).to_string()};
                } else {
                    bc::explorer::config::script config_contract(bin_script);

                    bc::chain::script redeem_script;
                    if (!redeem_script.from_data(bin_script, false, bc::chain::script::parse_mode::strict)) {
                        throw redeem_script_data_exception{"error occured when parse redeem script data."};
                    }

                    // walk the script and find potential public kes
                    std::set<data_chunk> script_pk_set;
                    auto f = [&script_pk_set](const operation& op) {
                        if ((op.code == opcode::special) && (op.data.size() == 33)) {
                            script_pk_set.insert(op.data);
                        }
                    };
                    std::for_each(redeem_script.operations.begin(), redeem_script.operations.end(), f);

                    std::map<std::string, std::string> pk_sig;
                    for (const auto& pk : script_pk_set) {
                        const payment_address script_pkh = wallet::ec_public(pk).to_payment_address();
                        auto acc_addr = blockchain.get_account_address(auth_.name, script_pkh.encoded());
                        if (!acc_addr) {
                            continue;
                        }
                        data_chunk public_key_data;
                        bc::endorsement&& edsig = sign(node, tx_, index, acc_addr->get_address(), config_contract, public_key_data);
                        pk_sig[encode_base16(public_key_data)] = encode_base16(edsig);
                    }

                    script_sig_map[index] = pk_sig;
                }
            } else {
                bc::explorer::config::script config_contract(output.script); // previous output script

                data_chunk public_key_data;
                bc::endorsement&& edsig = sign(node, tx_, index, address.encoded(), config_contract, public_key_data);

                // do script
                bc::chain::script ss;
                ss.operations.push_back({bc::chain::opcode::special, edsig});
                ss.operations.push_back({bc::chain::opcode::special, public_key_data});
                const bc::chain::script& contract = config_contract;
                // if pre-output script is deposit tx.
                if (contract.pattern() == bc::chain::script_pattern::pay_key_hash_with_lock_height) {
                    uint64_t lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(
                            contract.operations);
                    ss.operations.push_back({bc::chain::opcode::special, script_number(lock_height).data()});
                }

                // set input script of this tx
                tx_.inputs[index].script = ss;
            }
            index++;
        }// end for

    }

    if (script_sig_map.empty()) {
        // get raw tx
        //if (blockchain.validate_transaction(tx_)) {
        //    throw tx_validate_exception{"validate transaction failure"};
        //}

        jv_output = config::json_helper(get_api_version()).prop_list_of_rawtx(tx_, true);


    } else {
        for (auto iter1=script_sig_map.begin(); iter1 != script_sig_map.end(); ++iter1) {
            Json::Value pk_sig;
            for (auto iter2=iter1->second.begin(); iter2!=iter1->second.end(); ++iter2) {
                pk_sig[iter2->first] = iter2->second;
            }
            jv_output[std::to_string(iter1->first)] = pk_sig;
        }
    }

    return console_result::okay;
}
bc::endorsement signrawtx::sign(libbitcoin::server::server_node& node, tx_type tx_, const uint32_t& index, const std::string& address, const bc::explorer::config::script& config_contract, data_chunk& public_key_data)
{
    auto &blockchain = node.chain_impl();
    auto acc_addr = blockchain.get_account_address(auth_.name, address);

    if (!acc_addr)
        throw argument_legality_exception{"not own address " + address};

    // paramaters
    explorer::config::hashtype sign_type;
    uint8_t hash_type = (signature_hash_algorithm)sign_type;

    bc::explorer::config::ec_private config_private_key(acc_addr->get_prv_key(auth_.auth)); // address private key
    const ec_secret& private_key =    config_private_key;
    bc::wallet::ec_private ec_private_key(private_key, 0u, true);

    auto&& public_key = ec_private_key.to_public();
    public_key.to_data(public_key_data);

    const bc::chain::script& contract = config_contract;

    // gen sign
    bc::endorsement endorse;
    if (!bc::chain::script::create_endorsement(endorse, private_key,
            contract, tx_, index, hash_type))
    {
        throw tx_sign_exception{"signrawtx sign failure"};
    }

    return endorse;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

