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


#include <metaverse/explorer/extensions/node_method_wrapper.hpp>
#include <metaverse/explorer/extensions/commands/getwork.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ getwork *************************/

console_result getwork::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{

    administrator_required_checker(node, auth_.name, auth_.auth);

    std::string seed_hash;
    std::string header_hash;
    std::string boundary;

    auto& blockchain = node.chain_impl();
    auto& miner = node.miner();

    auto ret = miner.get_work(seed_hash, header_hash, boundary);

    if (ret) {
        Json::Value aroot;

        aroot.put("id", 1);
        aroot.put("jsonrpc", "1.0");
        
        Json::Value result;
        result.push_back(std::make_pair("", Json::Value().put("", header_hash)));
        result.push_back(std::make_pair("", Json::Value().put("", seed_hash)));
        result.push_back(std::make_pair("", Json::Value().put("", boundary)));

        aroot.add_child("result", result);
        pt::write_json(output, aroot);

        return console_result::okay;
    } else {
        output<<"Use command <setminingaccount> to set mining address.";
        return console_result::failure;
    }
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

