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
#include <metaverse/explorer/extensions/commands/getbestblockhash.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ getbestblockhash *************************/

console_result getbestblockhash::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{

    uint64_t height = 0;
    auto& blockchain = node.chain_impl();
    if(!blockchain.get_last_height(height))
        throw block_last_height_get_exception{"query last height failure."};

    auto&& height_str = std::to_string(height);
    const char* cmds[]{"fetch-header", "-t", height_str.c_str()};

    std::stringstream sout("");
    std::istringstream sin("");

    if (dispatch_command(3, cmds, sin, sout, sout) != console_result::okay) {
        throw block_hash_get_exception(sout.str());
    }
     
     
    relay_exception(sout);

    Json::Value header;
    sin.str(sout.str());
    pt::read_json(sin, header);

    auto&& blockhash = header.get<std::string>("result.hash");
    output<<blockhash;

    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

