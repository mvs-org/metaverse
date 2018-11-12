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
#include <metaverse/explorer/extensions/commands/signmultisigtx.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using payment_address = wallet::payment_address;

console_result signmultisigtx::invoke(
    Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto account = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    tx_type tx_ = argument_.transaction;

    // get all address of this account
    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if (!pvaddr) {
        throw address_list_empty_exception{"empty address list for this account."};
    }

    auto get_prikey = [&](const std::string& publickey) -> std::string {
        for (auto& each : *pvaddr) {
            auto prv_key = each.get_prv_key(auth_.auth);
            auto pub_key = ec_to_xxx_impl("ec-to-public", prv_key);
            if (publickey == pub_key) {
                return prv_key;
            }
        }
        return "";
    };

    std::string addr_prikey("");
    if (!option_.self_publickey.empty()) {
        addr_prikey = get_prikey(option_.self_publickey);

        if (addr_prikey.empty()) {
            throw pubkey_dismatch_exception(
                "public key " + option_.self_publickey + " is not owned by " + auth_.name);
        }
    }

    bc::chain::script input_script;
    bc::chain::script redeem_script;

    bool fullfilled = true;
    for (uint32_t index = 0; index < tx_.inputs.size(); ++index) {
        auto& each_input = tx_.inputs[index];
        input_script = each_input.script;

        if (chain::script_pattern::sign_multisig != input_script.pattern())
            continue;

        // 1. extract address from multisig payment script
        // zero sig1 sig2 ... encoded-multisig
        const auto& redeem_data = input_script.operations.back().data;
        if (redeem_data.empty()) {
            throw redeem_script_empty_exception{"empty redeem script."};
        }

        if (!redeem_script.from_data(redeem_data, false, bc::chain::script::parse_mode::strict)) {
            throw redeem_script_data_exception{"error occured when parse redeem script data."};
        }

        // Is the redeem script a standard pay (output) script?
        if (redeem_script.pattern() != chain::script_pattern::pay_multisig) {
            throw redeem_script_pattern_exception{"redeem script is not pay multisig pattern."};
        }

        const payment_address address(redeem_script, payment_address::mainnet_p2sh);
        auto hash_address = address.encoded(); // pay address

        // 2. get address prikey
        auto multisig_vec = account->get_multisig(hash_address);
        if (!multisig_vec || multisig_vec->empty()) {
            throw multisig_notfound_exception(hash_address + " multisig record not found.");
        }

        // signed, nothing to do (2 == zero + encoded-script)
        account_multisig acc_multisig_first = *(multisig_vec->begin());
        if (input_script.operations.size() >= acc_multisig_first.get_m() + 2) {
            continue;
        }

        std::string multisig_script = acc_multisig_first.get_multisig_script();
        if (multisig_script.empty()) {
            throw tx_sign_exception{"get_multisig_script get empty script."};
        }

        bool has_new_sign = false;
        for (const auto& acc_multisig : *multisig_vec) {
            if (!option_.self_publickey.empty() && option_.self_publickey != acc_multisig.get_pub_key()) {
                continue;
            }

            if (option_.self_publickey.empty()) {
                addr_prikey = get_prikey(acc_multisig.get_pub_key());
            }

            if (addr_prikey.empty()) {
                throw prikey_notfound_exception(
                    "The private key of " + acc_multisig.get_pub_key() + " not found.");
            }

            // prepare sign
            explorer::config::hashtype sign_type;
            uint8_t hash_type = (chain::signature_hash_algorithm)sign_type;

            bc::explorer::config::ec_private config_private_key(addr_prikey);
            bc::explorer::config::script config_contract(multisig_script);

            // gen sign
            bc::endorsement endorse;
            if (!bc::chain::script::create_endorsement(
                        endorse, config_private_key, config_contract, tx_, index, hash_type)) {
                throw tx_sign_exception{"get_input_sign sign failure"};
            }

            // insert endorse before multisig script
            auto position = input_script.operations.end();
            input_script.operations.insert(position - 1, {bc::chain::opcode::special, endorse});
            has_new_sign = true;
        }

        if (!has_new_sign && !option_.self_publickey.empty()) {
            throw tx_sign_exception{"has no multisig match self public key: " + option_.self_publickey};
        }

        // rearange signature order
        data_chunk data;
        chain::script script_encoded;
        script_encoded.from_string(multisig_script);

        bc::chain::script new_script;
        // insert zero
        new_script.operations.push_back(input_script.operations.front());

        /*
            "multisig-script" : "2 [ pubkey_1 ] [ pubkey_2 ] [ pubkey_3 ] 3 checkmultisig",
            "script" : "zero [ signature_1 ] [ signature_2 ] [ encoded-script ]",
        */
        // skip first "m" and last "n checkmultisig"
        auto multisig_start = script_encoded.operations.begin() + 1;
        auto multisig_end = script_encoded.operations.end() - 2;

        // skip first zero and last encoded-script
        auto script_op_start = input_script.operations.begin() + 1;
        auto script_op_end = input_script.operations.end() - 1;

        for (auto multisig_it = multisig_start; multisig_it != multisig_end; ++multisig_it) {
            for (auto script_op_it = script_op_start; script_op_it != script_op_end; ++script_op_it) {
                auto endorsement = script_op_it->data;
                const auto sighash_type = endorsement.back();
                auto distinguished = endorsement;
                distinguished.pop_back();

                ec_signature signature;
                // from validate_transaction.cpp handle_previous_tx
                auto strict = ((chain::get_script_context() & chain::script_context::bip66_enabled) != 0);
                if (!parse_signature(signature, distinguished, strict)) {
                    log::trace("multisig") << "failed to parse_signature! " << sighash_type;
                    continue;
                }

                if (chain::script::check_signature(signature, sighash_type, multisig_it->data,
                                                   script_encoded, tx_, index)) {
                    new_script.operations.push_back(*script_op_it);
                    break;
                }
            }

            if (new_script.operations.size() >= acc_multisig_first.get_m() + 1) {
                break;
            }
        }

        // insert encoded-script
        new_script.operations.push_back(input_script.operations.back());
        if (new_script.operations.size() < acc_multisig_first.get_m() + 2) {
            fullfilled = false;
        }

        // set input script of this tx
        each_input.script = new_script;
    }

    // output json
    if (get_api_version() <= 2) {
        jv_output = config::json_helper(get_api_version()).prop_list_of_rawtx(tx_, false, true);
    }
    else {
        jv_output = config::json_helper(get_api_version()).prop_list_of_rawtx(tx_, true);
    }

    if (option_.broadcast_flag /* TODO && fullfilled */) {
        log::trace("multisig") << "validate and broadcast multisig transaction." << tx_.to_string(1);

        if (blockchain.validate_transaction(tx_)) {
            throw tx_validate_exception{"validate transaction failure"};
        }

        if (blockchain.broadcast_transaction(tx_)) {
            throw tx_broadcast_exception{"broadcast transaction failure"};
        }
    }

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

