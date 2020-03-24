/**
 * Copyright (c) 2016-2020 mvs developers
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
#include <metaverse/explorer/extensions/commands/sendrawtx.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

bool sort_multi_sigs(tx_type& tx_) {
    bc::chain::script input_script;
    bc::chain::script redeem_script;

    for (uint32_t index = 0; index < tx_.inputs.size(); ++index) {
        auto &each_input = tx_.inputs[index];
        input_script = each_input.script;

        if (chain::script_pattern::sign_multisig != input_script.pattern())
            continue;

        // 1. extract address from multisig payment script
        // zero sig1 sig2 ... encoded-multisig
        const auto &redeem_data = input_script.operations.back().data;
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

        const wallet::payment_address address(redeem_script, wallet::payment_address::mainnet_p2sh);
        auto hash_address = address.encoded(); // pay address

        // rearange signature order
        data_chunk data;
        chain::script &script_encoded = redeem_script;
        //script_encoded.from_string(multisig_script);

        bc::chain::script new_script;
        // insert zero
        new_script.operations.push_back(input_script.operations.front());

        /*
            "multisig-script" : "2 [ pubkey_1 ] [ pubkey_2 ] [ pubkey_3 ] 3 checkmultisig",
            "script" : "zero [ signature_1 ] [ signature_2 ] [ encoded-script ]",
        */
        // skip first "m" and last "n checkmultisig"
        static constexpr auto op_1 = static_cast<uint8_t>(chain::opcode::op_1);
        const auto op_m = static_cast<uint8_t>(script_encoded.operations[0].code);
        if (op_m < op_1) {
            return false;
        }
        const auto num_m = op_m - op_1 + 1u;

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
        }

        // insert encoded-script
        new_script.operations.push_back(input_script.operations.back());
        if (new_script.operations.size() < num_m + 2) {
            return false;
        }

        // set input script of this tx
        each_input.script = new_script;
    }

    return true;
}

void check_forbidden_transaction(blockchain::block_chain_impl& blockchain, chain::transaction& tx)
{
    uint64_t last_height = 0;
    if (!blockchain.get_last_height(last_height)) {
        throw block_last_height_get_exception{"query last height failure."};
    }

    if (last_height >= pos_enabled_height) {
        for (auto& output : tx.outputs) {
            if (chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations)) {
                throw tx_validate_exception{"deposit is forbidden after PoS enabled."};
            }
        }
    }
}

console_result sendrawtx::invoke(Json::Value& jv_output,
                                 libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    tx_type tx_ = argument_.transaction;

    check_forbidden_transaction(blockchain, tx_);

    uint64_t outputs_etp_val = tx_.total_output_value();
    uint64_t inputs_etp_val = 0;
    if (!blockchain.get_tx_inputs_etp_value(tx_, inputs_etp_val))
        throw tx_validate_exception{"get transaction inputs etp value error!"};

    // check raw tx fee range
    if (inputs_etp_val <= outputs_etp_val) {
        throw tx_validate_exception{"no enough transaction fee"};
    }
    base_transfer_common::check_fee_in_valid_range(inputs_etp_val - outputs_etp_val);

    code ec = blockchain.validate_transaction(tx_);
    if (ec.value() != error::success) {
        if (!sort_multi_sigs(tx_)) {
            throw tx_validate_exception{"validate multi-sig transaction failure:" + ec.message()};
        }

        ec = blockchain.validate_transaction(tx_);
        if (ec.value() != error::success) {
            throw tx_validate_exception{"validate transaction failure: " + ec.message()};
        }
    }

    ec = blockchain.broadcast_transaction(tx_);
    if (ec.value() != error::success) {
        throw tx_broadcast_exception{"broadcast transaction failure: " + ec.message()};
    }

    if (get_api_version() <= 2) {
        jv_output["hash"] = encode_hash(tx_.hash());
    }
    else {
        jv_output = encode_hash(tx_.hash());
    }

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

