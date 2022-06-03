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
#include <metaverse/explorer/extensions/commands/createrawtx.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

console_result createrawtx::invoke(Json::Value& jv_output,
                                   libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();

    // check mychange
    std::string change_address;
    if (!option_.mychange_address.empty()) {
        change_address = get_address(option_.mychange_address, blockchain);
    }

    // check senders
    if (option_.senders.empty() && option_.utxos.empty()) {
        throw fromaddress_invalid_exception{"senders and utxos can not both be empty!"};
    }

    std::vector<std::string> senders;
    std::set<std::string> senders_set;
    std::string from_did;

    for (auto& each : option_.senders) {
        auto addr = get_address(each, blockchain);
        // filter script address
        if (blockchain.is_script_address(addr)) {
            throw fromaddress_invalid_exception{"invalid sender did/address " + each};
        }
        if (senders_set.insert(addr).second) {
            senders.emplace_back(addr);
            if (from_did.empty() && !blockchain.is_valid_address(each)) {
                from_did = each;
            }
        }
    }

    // from did is meaningful only when there is exact one sender
    if (!from_did.empty()) {
        if (!(senders.size() == 1 && option_.utxos.empty())) {
            from_did = "";
        }
    }

    auto type = static_cast<utxo_attach_type>(option_.type);

    if (type == utxo_attach_type::asset_transfer) {
        blockchain.uppercase_symbol(option_.symbol);

        // check asset symbol
        check_asset_symbol(option_.symbol);
    }

    // receiver
    std::vector<receiver_record> receivers;
    for ( auto& each : option_.receivers) {
        colon_delimited2_item<std::string, uint64_t> item(each);

        chain::attachment attach;
        auto addr = get_address(item.first(), attach, false, blockchain);
        if (!from_did.empty()) {
            attach.set_from_did(from_did);
            attach.set_version(DID_ATTACH_VERIFY_VERSION);
        }

        receiver_record record;
        record.target = addr;
        record.attach_elem = std::move(attach);
        record.type = type;
        record.symbol = option_.symbol;

        if (record.symbol.empty()) {
            record.amount = item.second(); // etp amount
            record.asset_amount = 0;
            if (!record.amount)
                throw argument_legality_exception{"invalid amount parameter " + each};
        }
        else {
            record.amount = 0;
            record.asset_amount = item.second();
            if (!record.asset_amount)
                throw argument_legality_exception{"invalid asset amount parameter " + each};
        }

        receivers.push_back(record);
    }

    if (receivers.empty()) {
        throw toaddress_invalid_exception{"receivers can not be empty!"};
    }

    std::shared_ptr<base_transfer_common> sp_send_helper;

    switch (type) {
    case utxo_attach_type::etp:
    case utxo_attach_type::asset_transfer: {
        sp_send_helper = std::make_shared<base_transaction_constructor>(
                             blockchain, type,
                             std::move(senders), std::move(receivers),
                             std::move(option_.symbol), std::move(change_address),
                             std::move(option_.message),
                             option_.fee, option_.locktime,
                             option_.include_input_script,
                             option_.utxo_min_confirm);
        break;
    }

    default: {
        throw argument_legality_exception{"invalid transaction type."};
        break;
    }
    }

    chain::history::list utxo_list;
    std::unordered_map<chain::input_point, uint32_t> utxo_seq_map; //((hash, index), sequence)
    for (const std::string& utxo : option_.utxos) {
        const auto utxo_stru = bc::split(utxo, ":");
        if ((utxo_stru.size() != 2) && (utxo_stru.size() != 3)) {
            throw argument_legality_exception{"invalid utxo: " + utxo};
        }

        const bc::config::hash256 hash = utxo_stru[0];

        uint64_t tx_height = 0;
        bc::chain::transaction tx;
        auto exist = blockchain.get_transaction_consider_pool(tx, tx_height, hash);
        if (!exist) {
            throw tx_notfound_exception{"transaction[" + utxo_stru[0] + "] does not exist!"};
        }

        const auto utxo_index = to_uint32_throw(utxo_stru[1], "wrong utxo index!");
        if ( !(utxo_index < tx.outputs.size()) ) {
            throw tx_notfound_exception{"output index[" + utxo_stru[1] + "] of transaction[" + utxo_stru[0] + "] is out of range!"};
        }
        if (utxo_stru.size() == 3) {
            if ((chain::get_script_context() & chain::script_context::bip112_enabled) == 0) {
                throw argument_legality_exception{"invalid utxo: " + utxo + ", lock sequence(bip112) is not enabled"};
            }
            chain::input_point utxo_point(hash, utxo_index);
            if (utxo_seq_map.count(utxo_point)) {
                throw argument_legality_exception{"duplicate utxo: " + utxo};
            }
            const auto utxo_sequence = to_uint32_throw(utxo_stru[2], "wrong utxo sequence!");
            utxo_seq_map[utxo_point] = utxo_sequence;
        }

        chain::history h;
        h.output.hash = tx.hash();
        h.output.index = utxo_index;
        h.output_height = tx_height;
        h.value = tx.outputs[utxo_index].value;
        utxo_list.push_back(h);
    }

    if (!utxo_list.empty()) {
        sp_send_helper->use_specified_rows(true);  //customlized rows, ignore is_payment_satisfied;
        sp_send_helper->sync_fetchutxo("", "", base_transfer_common::FILTER_ALL, utxo_list);
    }

    sp_send_helper->exec();

    auto&& tx = sp_send_helper->get_transaction();


    // set sequence
    if (!utxo_seq_map.empty()) {
        for (auto& input : tx.inputs) {
            if (!utxo_seq_map.count(input.previous_output)) {
                continue;
            }
            if ((input.sequence & bc::relative_locktime_disabled) ||
                (input.sequence & bc::relative_locktime_mask) == 0) {
                input.sequence = utxo_seq_map[input.previous_output];
            }
        }
    }

    // output json
    jv_output = config::json_helper(get_api_version()).prop_list_of_rawtx(tx, false);

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

