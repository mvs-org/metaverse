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
    blockchain.uppercase_symbol(option_.symbol);

    if (!option_.mychange_address.empty() && !blockchain.is_valid_address(option_.mychange_address))
        throw toaddress_invalid_exception{"invalid address " + option_.mychange_address};

    // check senders
    if (option_.senders.empty()) {
        throw fromaddress_invalid_exception{"senders can not be empty!"};
    }

    for (auto& each : option_.senders) {
        if (!blockchain.is_valid_address(each)) {
            throw fromaddress_invalid_exception{"invalid sender address " + each};
        }

        // filter script address
        if (blockchain.is_script_address(each)) {
            throw fromaddress_invalid_exception{"invalid sender address " + each};
        }
    }

    auto type = static_cast<utxo_attach_type>(option_.type);

    if (type == utxo_attach_type::deposit) {
        if (!option_.symbol.empty()) {
            throw argument_legality_exception{"not deposit asset " + option_.symbol};
        }

        if (option_.receivers.size() != 1) {
            throw argument_legality_exception{"only support deposit on one address!"};
        }
    }
    else if (type == utxo_attach_type::asset_transfer) {
        blockchain.uppercase_symbol(option_.symbol);

        // check asset symbol
        check_asset_symbol(option_.symbol);
    }

    // receiver
    receiver_record record;
    std::vector<receiver_record> receivers;
    for ( auto& each : option_.receivers) {
        colon_delimited2_item<std::string, uint64_t> item(each);
        record.target = item.first();
        // address check
        if (!blockchain.is_valid_address(record.target)) {
            throw toaddress_invalid_exception{"invalid receiver address " + record.target};
        }

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

        record.type = type;
        receivers.push_back(record);
    }

    if (receivers.empty()) {
        throw toaddress_invalid_exception{"receivers can not be empty!"};
    }

    std::shared_ptr<base_transfer_common> sp_send_helper;

    switch (type) {
        case utxo_attach_type::etp:
        case utxo_attach_type::asset_transfer: {
            sp_send_helper = std::make_shared<base_transaction_constructor>(blockchain, type,
                             std::move(option_.senders), std::move(receivers),
                             std::move(option_.symbol), std::move(option_.mychange_address),
                             std::move(option_.message), option_.fee);
            break;
        }

        case utxo_attach_type::deposit: {
            sp_send_helper = std::make_shared<depositing_etp_transaction>(blockchain, type,
                             std::move(option_.senders), std::move(receivers),
                             option_.deposit, std::move(option_.mychange_address),
                             std::move(option_.message), option_.fee);
            break;
        }

        default: {
            throw argument_legality_exception{"invalid transaction type."};
            break;
        }
    }

    sp_send_helper->exec();

    auto&& tx = sp_send_helper->get_transaction();

    // output json
    std::ostringstream tx_buf;
    tx_buf << config::transaction(tx);
    if (get_api_version() <= 2) {
        jv_output["hex"] = tx_buf.str();
    }
    else {
        // TODO support restful API format
        jv_output["raw"] = tx_buf.str();
    }

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

