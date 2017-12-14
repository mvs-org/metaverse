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


#include <jsoncpp/json/json.h>
#include <metaverse/explorer/extensions/node_method_wrapper.hpp>
#include <metaverse/explorer/extensions/commands/stopmining.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ stopmining *************************/

console_result stopmining::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    administrator_required_checker(node, auth_.name, auth_.auth);

    auto& miner = node.miner();
    auto ret = miner.stop();
    if (ret) {
        output<<"mining stoped.";
        return console_result::okay;
    } else {
        output<<"mining stoped got error.";
        return console_result::failure;
    }
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

