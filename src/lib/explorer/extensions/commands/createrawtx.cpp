/**
 * Copyright (c) 2016-2017 mvs developers 
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

#include <boost/property_tree/ptree.hpp>      
#include <boost/property_tree/json_parser.hpp>

#include <metaverse/bitcoin.hpp>
#include <metaverse/client.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/callback_state.hpp>
#include <metaverse/explorer/display.hpp>
#include <metaverse/explorer/prop_tree.hpp>
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/createrawtx.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

console_result createrawtx::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.uppercase_symbol(option_.symbol);

    tx_type tx_;
    
    if (!option_.mychange_address.empty() && !blockchain.is_valid_address(option_.mychange_address))
        throw toaddress_invalid_exception{std::string("invalid address ") + option_.mychange_address};
    // senders check
    for (auto& each : option_.senders){
        // filter script address
        if(blockchain.is_script_address(each))
            throw fromaddress_invalid_exception{std::string("invalid address ") + each};
    }

    auto type = static_cast<utxo_attach_type>(option_.type);
    // receiver
    receiver_record record;
    std::vector<receiver_record> receivers;
    for( auto& each : option_.receivers){
        colon_delimited2_item<std::string, uint64_t> item(each);
        record.target = item.first();
        // address check
        if (!blockchain.is_valid_address(record.target))
            throw toaddress_invalid_exception{std::string("invalid address ") + record.target};
        record.symbol = option_.symbol;
        if(record.symbol.empty()) {
            record.amount = item.second(); // etp amount
            record.asset_amount = 0;
            if (!record.amount)
                throw argument_legality_exception{std::string("invalid amount parameter ") + each};
        } else {
            record.amount = 0;
            record.asset_amount = item.second();
            if (!record.asset_amount)
                throw argument_legality_exception{std::string("invalid asset amount parameter ") + each};
        }
        record.type = type;
        receivers.push_back(record);
    }

    if((type == utxo_attach_type::etp) || (type == utxo_attach_type::asset_transfer)) {
        auto send_helper = base_transaction_constructor(blockchain, type, 
                std::move(option_.senders), std::move(receivers), std::move(option_.symbol), std::move(option_.mychange_address), 
                std::move(option_.message), option_.fee);

        send_helper.exec();
        tx_ = send_helper.get_transaction();
    } else {
        throw argument_legality_exception{"invalid transaction type."};
    }
    

    pt::ptree aroot;
    std::ostringstream tx_buf;
    tx_buf << config::transaction(tx_);
    aroot.put("hex", tx_buf.str());
    
    pt::write_json(output, aroot);

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

