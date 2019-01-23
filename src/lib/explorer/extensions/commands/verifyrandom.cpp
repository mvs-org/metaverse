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


#include <metaverse/explorer/extensions/commands/verifyrandom.hpp>
#include <metaverse/node/p2p_node.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

/************************ verifyrandom *************************/

console_result verifyrandom::invoke(Json::Value& jv_output,
                                   libbitcoin::server::server_node& node)
{
    auto rand_hash = argument_.random_hash;
    auto rand_num = argument_.random_num;

    auto chunk = to_chunk(std::to_string(rand_num));
    auto calc_hash = sha3(chunk).hex();

    auto is_valid = calc_hash == rand_hash;

    Json::Value result;

    result["random_num"] = rand_num;
    result["random_hash"] = rand_hash;
    result["is_valid"] = is_valid;

    jv_output = result;

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

