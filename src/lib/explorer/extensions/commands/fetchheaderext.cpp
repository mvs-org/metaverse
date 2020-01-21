/**
 * Copyright (c) 2019-2020 mvs developers
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


#include <metaverse/explorer/extensions/commands/fetchheaderext.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/json_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ fetchheaderext *************************/

console_result fetchheaderext::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if(argument_.number.empty())
        throw block_height_get_exception{"Block number or earliest, latest, pending is needed"};

    chain::header block_header;

    auto& miner = node.miner();
    auto ret = miner.get_block_header(block_header, argument_.number);

    auto& aroot = jv_output;

    if (ret) {

        auto&& result = config::json_helper(get_api_version()).prop_list(block_header);

        if (get_api_version() == 1) {
            aroot["result"] = result;
        } else {
            aroot = result;
        }

    } else {
        throw block_height_get_exception{"get block header on height " + argument_.number + " failed."};
    }

    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

