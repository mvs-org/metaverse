/**
 * Copyright (c) 2016-2021 mvs developers
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

std::string parse_private_key(const std::string& raw_str)
{
    // decode wif
    bc::wallet::ec_private prv_key(raw_str);
    if (true == (bool)prv_key) {
        const ec_secret& secret = prv_key;
        return encode_base16(secret);
    }

    return raw_str;
}

std::string signrawtx::get_private_key(const std::vector<std::string>& keys, const std::string& address)
{
    for (auto& key : keys) {
        bc::explorer::config::ec_private cfg_prv(key);
        const ec_secret& secret = cfg_prv;

        bc::wallet::ec_private wlt_prv(secret);
        auto payment_address = wlt_prv.to_payment_address();
        if (payment_address.encoded() == address) {
            return key;
        }
    }

    throw argument_legality_exception{"No private key matches address " + address};
}

console_result signrawtx::invoke(Json::Value& jv_output,
                                 bc::server::server_node& node)
{
    if (option_.private_keys.empty() && (auth_.name.empty() || auth_.auth.empty())) {
        throw argument_legality_exception{"Missing account/password or private key!"};
    }

    auto &blockchain = node.chain_impl();
    std::shared_ptr<chain::account> acc(nullptr);
    std::vector<std::string> private_keys;
    if (option_.private_keys.empty()) {
        acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    }
    else {
        for (auto& key : option_.private_keys) {
            private_keys.push_back(parse_private_key(key));
        }
    }

    tx_type tx_ = argument_.transaction;

    std::map<uint32_t, std::map<std::string, std::string> > script_sig_map;

    // sign tx
    {
        uint32_t index = 0;

        for (auto& input : tx_.inputs) {
            auto prev_output_script = get_prev_output_script(blockchain, input);

            // get address private key
            auto address = payment_address::extract(prev_output_script);

            // script address : maybe multisig
            if (!address || (address.version() == payment_address::mainnet_p2sh)) {
                if (acc == nullptr) {
                    throw argument_legality_exception{"Please use account/password to sign pay-to-script raw tx."};
                }

                const auto script_vec = acc->get_script(address.encoded());
                if (!script_vec || script_vec->empty()) {
                    throw argument_legality_exception{"invalid script: " + config::script(prev_output_script).to_string()};
                }

                const auto& script = script_vec->begin();

                // watch-only address
                const data_chunk& bin_script = script->get_script();
                if (bin_script.empty()) {
                    throw argument_legality_exception{"watch-only script address: " + config::script(prev_output_script).to_string()};
                }
                else {
                    bc::explorer::config::script config_contract(bin_script);

                    bc::chain::script redeem_script;
                    if (!redeem_script.from_data(bin_script, false, bc::chain::script::parse_mode::strict)) {
                        throw redeem_script_data_exception{"error occured when parse redeem script data."};
                    }

                    // walk the script and find potential public kes
                    std::set<data_chunk> script_pk_set;
                    auto f = [&script_pk_set](const chain::operation& op) {
                        if ((op.code == chain::opcode::special) && (op.data.size() == 33)) {
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

                        std::string prv_key_str = acc_addr->get_prv_key(auth_.auth);;

                        data_chunk public_key_data;
                        bc::endorsement&& edsig = sign(prv_key_str, tx_, index, config_contract, public_key_data);
                        pk_sig[encode_base16(public_key_data)] = encode_base16(edsig);
                    }

                    script_sig_map[index] = pk_sig;
                }
            }
            else {
                std::string prv_key_str;
                if (acc) {
                    prv_key_str = get_private_key(blockchain, address.encoded());
                }
                else {
                    prv_key_str = get_private_key(private_keys, address.encoded());
                }

                bc::explorer::config::script config_contract(prev_output_script);

                data_chunk public_key_data;
                bc::endorsement&& edsig = sign(prv_key_str, tx_, index, config_contract, public_key_data);

                // do script
                bc::chain::script ss;
                ss.operations.push_back({bc::chain::opcode::special, edsig});
                ss.operations.push_back({bc::chain::opcode::special, public_key_data});
                const bc::chain::script& contract = config_contract;
                // if prev_output_script is deposit tx.
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
    }
    else {
        for (auto iter1 = script_sig_map.begin(); iter1 != script_sig_map.end(); ++iter1) {
            Json::Value pk_sig;
            for (auto iter2 = iter1->second.begin(); iter2 != iter1->second.end(); ++iter2) {
                pk_sig[iter2->first] = iter2->second;
            }
            jv_output[std::to_string(iter1->first)] = pk_sig;
        }
    }

    return console_result::okay;
}

std::string signrawtx::get_private_key(blockchain::block_chain_impl& blockchain, const std::string& address)
{
    auto acc_addr = blockchain.get_account_address(auth_.name, address);
    if (!acc_addr) {
        throw argument_legality_exception{"Address " + address + " is not owned."};
    }

    return acc_addr->get_prv_key(auth_.auth);
}

chain::script signrawtx::get_prev_output_script(
    blockchain::block_chain_impl& blockchain, const chain::input& input) const
{
    if (option_.offline) {
        if (input.script.is_valid()) {
            return input.script;
        }

        throw argument_legality_exception{"Sign rawtx offline must specify the rawtx's input script!"};
    }

    chain::transaction tx_temp;
    uint64_t tx_height;

    if (!blockchain.get_transaction_consider_pool(
        tx_temp, tx_height, input.previous_output.hash)) {
        throw argument_legality_exception{
            "invalid transaction hash " + encode_hash(input.previous_output.hash)};
    }

    auto output = tx_temp.outputs.at(input.previous_output.index);
    return output.script;
}

bc::endorsement signrawtx::sign(
    const std::string& prv_key_str,
    tx_type tx_,
    const uint32_t& index,
    const bc::explorer::config::script& config_contract,
    data_chunk& public_key_data)
{
    // paramaters
    explorer::config::hashtype sign_type;
    uint8_t hash_type = (chain::signature_hash_algorithm)sign_type;

    bc::explorer::config::ec_private config_private_key(prv_key_str);
    const ec_secret& secret = config_private_key;

    bc::wallet::ec_private ec_private_key(secret);
    auto&& public_key = ec_private_key.to_public();
    public_key.to_data(public_key_data);

    const bc::chain::script& contract = config_contract;

    // gen sign
    bc::endorsement endorse;
    if (!bc::chain::script::create_endorsement(endorse, secret,
            contract, tx_, index, hash_type))
    {
        throw tx_sign_exception{"signrawtx sign failure"};
    }

    return endorse;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

