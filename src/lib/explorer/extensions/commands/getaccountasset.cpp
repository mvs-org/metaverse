/**
 * Copyright (c) 2016-2020 mvs developers
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
#include <metaverse/explorer/extensions/commands/getaccountasset.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ getaccountasset *************************/

console_result getaccountasset::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    if (!argument_.symbol.empty()) {
        // check asset symbol
        check_asset_symbol(argument_.symbol);
    }

    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr)
        throw address_list_nullptr_exception{"nullptr for address list"};

    std::string json_key;
    Json::Value json_value;
    auto json_helper = config::json_helper(get_api_version());

    if (option_.is_cert || !option_.cert_type.empty()) { // only get asset certs
        json_key = "assetcerts";

        chain::asset_cert_type cert_type = asset_cert_ns::none;
        if (!option_.cert_type.empty()) {
            cert_type = check_cert_type_name(option_.cert_type, true);
        }

        auto sh_vec = std::make_shared<chain::asset_cert::list>();
        for (auto& each : *pvaddr){
            sync_fetch_asset_cert_balance(
                each.get_address(), argument_.symbol, blockchain, sh_vec, cert_type);
        }

        std::sort(sh_vec->begin(), sh_vec->end());
        for (auto& elem: *sh_vec) {
            Json::Value asset_cert = json_helper.prop_list(elem);
            json_value.append(asset_cert);
        }
    }
    else if (option_.deposited) {
        json_key = "assets";
        auto sh_vec = std::make_shared<chain::asset_deposited_balance::list>();

        // get address unspent asset balance
        std::string addr;
        for (auto& each : *pvaddr){
            sync_fetch_asset_deposited_balance(each.get_address(), blockchain, sh_vec);
        }

        std::sort(sh_vec->begin(), sh_vec->end());

        for (auto& elem: *sh_vec) {
            auto& symbol = elem.symbol;
            if (!argument_.symbol.empty() && argument_.symbol != symbol)
                continue;

            auto issued_asset = blockchain.get_issued_asset(symbol);
            if (!issued_asset) {
                continue;
            }

            Json::Value asset_data = json_helper.prop_list(elem, *issued_asset, true);
            asset_data["status"] = "unspent";
            json_value.append(asset_data);
        }
    }
    else {
        json_key = "assets";
        auto sh_vec = std::make_shared<chain::asset_balances::list>();

        // 1. get asset in blockchain
        // get address unspent asset balance
        std::string addr;
        for (auto& each : *pvaddr){
            sync_fetch_asset_balance(each.get_address(), false, blockchain, sh_vec);
        }

        std::sort(sh_vec->begin(), sh_vec->end());
        for (auto& elem: *sh_vec) {
            auto& symbol = elem.symbol;
            if (!argument_.symbol.empty() && argument_.symbol != symbol)
                continue;
            auto issued_asset = blockchain.get_issued_asset(symbol);
            if (!issued_asset) {
                continue;
            }
            Json::Value asset_data = json_helper.prop_list(elem, *issued_asset);
            asset_data["status"] = "unspent";
            json_value.append(asset_data);
        }

        // 2. get asset in local database
        // shoudl filter all issued asset which be stored in local account asset database
        auto sh_unissued = blockchain.get_account_unissued_assets(auth_.name);
        for (auto& elem: *sh_unissued) {
            auto& symbol = elem.detail.get_symbol();
            // symbol filter
            if(!argument_.symbol.empty() && argument_.symbol !=  symbol)
                continue;

            Json::Value asset_data = json_helper.prop_list(elem.detail, false);
            asset_data["status"] = "unissued";
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
        if (json_value.isNull())
            json_value.resize(0);

        jv_output = json_value;
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

