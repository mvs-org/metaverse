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
#include <metaverse/explorer/extensions/miner/getwork.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."

/************************ getwork *************************/

console_result getwork::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain,
        bc::consensus::miner& miner)
{
    std::string seed_hash;
    std::string header_hash;
    std::string boundary;
    auto ret = miner.get_work(seed_hash, header_hash, boundary);

    if (ret) {
        pt::ptree aroot;

        aroot.put("id", 1);
        aroot.put("jsonrpc", "1.0");
        
        pt::ptree result;
        result.push_back(std::make_pair("", pt::ptree().put("", header_hash)));
        result.push_back(std::make_pair("", pt::ptree().put("", seed_hash)));
        result.push_back(std::make_pair("", pt::ptree().put("", boundary)));

        aroot.add_child("result", result);
        pt::write_json(output, aroot);

        return console_result::okay;
    } else {
        output<<"no address setting.";
        return console_result::failure;
    }
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

