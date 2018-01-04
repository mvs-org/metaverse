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
#include <metaverse/explorer/extensions/commands/sendmore.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


console_result sendmore::invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if (!argument_.mychange_address.empty() && !blockchain.is_valid_address(argument_.mychange_address))
        throw toaddress_invalid_exception{std::string("invalid address!") + argument_.mychange_address};
    //if (!blockchain.get_account_address(auth_.name, argument_.mychange_address))
        //throw argument_legality_exception{argument_.mychange_address + std::string(" not owned to ") + auth_.name};
    // receiver
    receiver_record record;
    std::vector<receiver_record> receiver;
    
    for( auto& each : argument_.receivers){
        colon_delimited2_item<std::string, uint64_t> item(each);
        record.target = item.first();
        // address check
        if (!blockchain.is_valid_address(record.target))
            throw toaddress_invalid_exception{std::string("invalid address!") + record.target};
        record.symbol = "";
        record.amount = item.second();
        if (!record.amount)
            throw argument_legality_exception{std::string("invalid amount parameter!") + each};
        record.asset_amount = 0;
        record.type = utxo_attach_type::etp; // attach not used so not do attah init
        receiver.push_back(record);
    }
    auto send_helper = sending_etp_more(*this, blockchain, std::move(auth_.name), std::move(auth_.auth), 
            "", std::move(receiver), std::move(argument_.mychange_address), argument_.fee);
    
    send_helper.exec();

    // json output
    auto tx = send_helper.get_transaction();
     jv_output =  config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

