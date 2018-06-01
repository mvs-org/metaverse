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
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/listmits.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ listmits *************************/

console_result listmits::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();

    std::shared_ptr<identifiable_asset::list> sh_vec;
    if (auth_.name.empty()) {
        // no account -- list whole assets in blockchain
        sh_vec = blockchain.get_registered_identifiable_assets();
    }
    else {
        // list asset owned by account
        sh_vec = blockchain.get_account_identifiable_assets(auth_.name);
    }

    Json::Value json_value;
    auto json_helper = config::json_helper(get_api_version());
    if (nullptr != sh_vec) {
        std::sort(sh_vec->begin(), sh_vec->end());
        for (auto& elem : *sh_vec) {
            Json::Value asset_data = json_helper.prop_list(elem);
            json_value.append(asset_data);
        }
    }

    if (get_api_version() == 1 && json_value.isNull()) { //compatible for v1
        jv_output["mits"] = "";
    }
    else {
        jv_output["mits"] = json_value;
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
