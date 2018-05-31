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
#include <metaverse/explorer/extensions/commands/getdid.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

console_result getdid::invoke (Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    Json::Value json_value;
    std::string json_key;
    
    auto& blockchain = node.chain_impl();

    if(option_.symbol.empty())
    {
        json_key = "dids";

        auto sh_vec = blockchain.get_registered_dids();

        // add blockchain dids
        for (auto& elem: *sh_vec) 
            json_value.append(elem.get_symbol());  
    }
    else
    {
        json_key = "addresses";
        // check did symbol
        check_did_symbol(option_.symbol);

        // check did exists
        if (!blockchain.is_did_exist(option_.symbol))
            throw did_symbol_notfound_exception{"did symbol does not exist on the blockchain"};

        auto blockchain_dids = blockchain.get_did_history_addresses(option_.symbol);
        if (blockchain_dids) {
            Json::Value did_data;
            for (auto &did : *blockchain_dids){
                did_data["address"] = did.get_did().get_address();
                did_data["status"] = did.get_status_string();
                json_value.append(did_data);
            }
        }


    }


    if (get_api_version() == 1 && json_value.isNull()) { //compatible for v1
        jv_output[json_key] = "";
    }
    else {
        jv_output[json_key] = json_value;
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

