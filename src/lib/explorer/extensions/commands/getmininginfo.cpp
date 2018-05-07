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
#include <metaverse/explorer/extensions/node_method_wrapper.hpp>
#include <metaverse/explorer/extensions/commands/getmininginfo.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ getmininginfo *************************/

console_result getmininginfo::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    administrator_required_checker(node, auth_.name, auth_.auth);

    auto& aroot = jv_output;
    Json::Value info;

    auto& miner = node.miner();

    uint64_t height, rate;
    std::string difficulty;
    bool is_mining;

    miner.get_state(height, rate, difficulty, is_mining);
    info["is-mining"] = is_mining;
    info["height"] += height;
    info["rate"] += rate;
    info["difficulty"] = difficulty;
    aroot["mining-info"] = info;

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

