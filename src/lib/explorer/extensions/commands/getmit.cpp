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
#include <metaverse/explorer/extensions/commands/getmit.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

using namespace bc::explorer::config;

/************************ getmit *************************/

console_result getmit::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();

    if (!argument_.symbol.empty()) {
        // check symbol
        check_mit_symbol(argument_.symbol);
    }

    Json::Value json_value;
    auto json_helper = config::json_helper(get_api_version());

    bool is_list = true;
    if (argument_.symbol.empty()) {
        auto sh_vec = blockchain.get_registered_mits();
        std::sort(sh_vec->begin(), sh_vec->end());
        for (auto& elem : *sh_vec) {
            json_value.append(elem.get_symbol());
        }
    }
    else {
        if (option_.show_history) {
            auto sh_vec = blockchain.get_mit_history(argument_.symbol);
            for (auto& elem : *sh_vec) {
                Json::Value asset_data = json_helper.prop_list(elem, true);
                asset_data["did"] = blockchain.get_did_from_address(elem.get_address());
                json_value.append(asset_data);
            }
        }
        else {
            auto asset = blockchain.get_registered_mit(argument_.symbol);
            if (nullptr != asset) {
                json_value = json_helper.prop_list(*asset);
                json_value["did"] = blockchain.get_did_from_address(asset->get_address());
            }
        }
    }

    jv_output["mits"] = json_value;

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

