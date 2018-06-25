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

console_result signrawtx::invoke(Json::Value& jv_output,
                                 libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    tx_type tx_ = argument_.transaction;

    // sign tx
    {
        uint32_t index = 0;
        chain::transaction tx_temp;
        uint64_t tx_height;

        for (auto& fromeach : tx_.inputs) {
            if (!(blockchain.get_transaction(fromeach.previous_output.hash, tx_temp, tx_height)))
                throw argument_legality_exception{"invalid transaction hash " + encode_hash(fromeach.previous_output.hash)};

            auto output = tx_temp.outputs.at(fromeach.previous_output.index);

            // get address private key
            auto address = payment_address::extract(output.script);
            if (!address || (address.version() == 0x5)) // script address : maybe multisig
                throw argument_legality_exception{"invalid script " + config::script(output.script).to_string()};

            auto acc_addr = blockchain.get_account_address(auth_.name, address.encoded());

            if (!acc_addr)
                throw argument_legality_exception{"not own address " + address.encoded()};

            // paramaters
            explorer::config::hashtype sign_type;
            uint8_t hash_type = (signature_hash_algorithm)sign_type;

            bc::explorer::config::ec_private config_private_key(acc_addr->get_prv_key(auth_.auth)); // address private key
            const ec_secret& private_key =    config_private_key;
            bc::wallet::ec_private ec_private_key(private_key, 0u, true);

            bc::explorer::config::script config_contract(output.script); // previous output script
            const bc::chain::script& contract = config_contract;

            // gen sign
            bc::endorsement endorse;
            if (!bc::chain::script::create_endorsement(endorse, private_key,
                    contract, tx_, index, hash_type))
            {
                throw tx_sign_exception{"signrawtx sign failure"};
            }

            // do script
            auto&& public_key = ec_private_key.to_public();
            data_chunk public_key_data;
            public_key.to_data(public_key_data);
            bc::chain::script ss;
            ss.operations.push_back({bc::chain::opcode::special, endorse});
            ss.operations.push_back({bc::chain::opcode::special, public_key_data});

            // if pre-output script is deposit tx.
            if (contract.pattern() == bc::chain::script_pattern::pay_key_hash_with_lock_height) {
                uint64_t lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(
                                           contract.operations);
                ss.operations.push_back({bc::chain::opcode::special, script_number(lock_height).data()});
            }

            // set input script of this tx
            tx_.inputs[index].script = ss;
            index++;
        } // end for
    }

    // get raw tx
    if (blockchain.validate_transaction(tx_)) {
        throw tx_validate_exception{"validate transaction failure"};
    }

    std::ostringstream tx_buf;
    tx_buf << config::transaction(tx_);
    jv_output["hash"] = encode_hash(tx_.hash());
    jv_output["hex"] = tx_buf.str();

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

