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
#include <metaverse/explorer/extensions/commands/getaddressasset.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ getaddressasset *************************/

console_result getaddressasset::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    const auto address = get_address(argument_.address, blockchain);
    if (address.empty()) {
        throw address_invalid_exception{"invalid address!"};
    }

    if (!option_.symbol.empty()) {
        // check asset symbol
        check_asset_symbol(option_.symbol);
    }

    std::string json_key;
    Json::Value json_value;
    auto json_helper = config::json_helper(get_api_version());;

    if (option_.is_cert) { // only get asset certs
        json_key = "assetcerts";

        auto sh_vec = std::make_shared<asset_cert::list>();
        sync_fetch_asset_cert_balance(address, "", blockchain, sh_vec);
        std::sort(sh_vec->begin(), sh_vec->end());
        for (auto& elem: *sh_vec) {
            if (!option_.symbol.empty() && option_.symbol != elem.get_symbol())
                continue;

            Json::Value asset_cert = json_helper.prop_list(elem);
            json_value.append(asset_cert);
        }
    }
    else if (option_.deposited) {
        json_key = "assets";

        auto sh_vec = std::make_shared<asset_deposited_balance::list>();
        sync_fetch_asset_deposited_balance(address, blockchain, sh_vec);
        std::sort(sh_vec->begin(), sh_vec->end());

        for (auto& elem: *sh_vec) {
            if (!option_.symbol.empty() && option_.symbol != elem.symbol)
                continue;

            auto issued_asset = blockchain.get_issued_asset(elem.symbol);
            if (!issued_asset) {
                continue;
            }

            Json::Value asset_data = json_helper.prop_list(elem, *issued_asset);
            asset_data["status"] = "unspent";
            json_value.append(asset_data);
        }
    }
    else {
        json_key = "assets";

        auto sh_vec = std::make_shared<asset_balances::list>();
        sync_fetch_asset_balance(address, true, blockchain, sh_vec);
        std::sort(sh_vec->begin(), sh_vec->end());
        for (auto& elem: *sh_vec) {
            if (!option_.symbol.empty() && option_.symbol != elem.symbol)
                continue;

            auto issued_asset = blockchain.get_issued_asset(elem.symbol);
            if (!issued_asset) {
                continue;
            }
            Json::Value asset_data = json_helper.prop_list(elem, *issued_asset);
            asset_data["status"] = "unspent";
            json_value.append(asset_data);
        }
    }

    if (get_api_version() == 1 && json_value.isNull()) { //compatible for v1
        jv_output[json_key] = "";
    }
    else if (get_api_version() <= 2) {
        jv_output[json_key] = json_value;
    }
    else {
        if(json_value.isNull())
            json_value.resize(0);

        jv_output = json_value;
    }

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

