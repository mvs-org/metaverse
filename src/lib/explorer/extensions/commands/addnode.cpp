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
#include <metaverse/explorer/extensions/commands/addnode.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/node_method_wrapper.hpp>
#include <metaverse/bitcoin/config/authority.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ addnode *************************/

console_result addnode::invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node)
{
    //jv_output["message"] = IN_DEVELOPING;

    administrator_required_checker(node, auth_.name, auth_.auth);
    auto& root = jv_output;

    auto address = libbitcoin::config::authority(argument_.address).to_network_address();

    code errcode;
    auto handler = [&errcode](const code& ec){
        errcode = ec;
    };

    if (argument_.command == "add") {
        node.store(address, handler);
    } else if (argument_.command == "remove") {
        node.remove(address, handler);
    } else {
        jv_output["message"] = std::string("invalid command[") + argument_.command + "].";
        return console_result::okay;
    }

    jv_output["message"] = "done.";

    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

