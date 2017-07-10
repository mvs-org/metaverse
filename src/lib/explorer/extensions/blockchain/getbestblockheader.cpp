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
#include <metaverse/explorer/extensions/blockchain/getbestblockheader.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."

/************************ getbestblockheader *************************/

console_result getbestblockheader::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    uint64_t height = 0;
    auto& blockchain = node.chain_impl();
    if(!blockchain.get_last_height(height))
        throw block_height_get_exception{"query last height failure."};

    auto&& height_str = std::to_string(height);
    const char* cmds[]{"fetch-header", "-t", height_str.c_str()};

    std::ostringstream sout("");
    std::istringstream sin("");

	if (dispatch_command(3, cmds, sin, sout, sout) != console_result::okay) {
		throw get_block_header_get_exception(sout.str());
	}
	std::pair<uint32_t, std::string> ex_pair;
	std::stringstream ex_stream;
	ex_stream.str(sout.str());
	if (capture_excode(ex_stream, ex_pair) == console_result::okay) {
		throw explorer_exception(ex_pair.first, ex_pair.second);
	}

    output<<sout.str();

    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

