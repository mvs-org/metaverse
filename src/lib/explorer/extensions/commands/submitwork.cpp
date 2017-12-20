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


#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/submitwork.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ submitwork *************************/

console_result submitwork::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& miner = node.miner();
    auto ret = miner.put_result(argument_.nounce, argument_.mix_hash, argument_.header_hash);
    Json::Value root;

    root["id"] = "1";
    root["jsonrpc"] = "1.0";

    if (ret) {
        root["result"] = "true"; // boost json parser output as string, for compatible.
        output << root.toStyledString();
        return console_result::okay;
    } else {
        root["result"] = "false";
        output << root.toStyledString();
        return console_result::failure;
    }
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

