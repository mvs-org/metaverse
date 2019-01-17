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
#include <metaverse/explorer/extensions/commands/listassets.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ listassets *************************/

console_result listassets::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();

    std::string json_key;
    Json::Value json_value;

    auto json_helper = config::json_helper(get_api_version());

    if (option_.is_cert || !option_.cert_type.empty()) { // only get asset certs
        json_key = "assetcerts";

        asset_cert_type cert_type = asset_cert_ns::none;
        if (!option_.cert_type.empty()) {
            cert_type = check_cert_type_name(option_.cert_type, true);
        }

        if (auth_.name.empty()) { // no account -- list whole asset certs in blockchain
            if (cert_type == asset_cert_ns::witness) {
                auto bus_vec = blockchain.get_issued_witness_certs();
                for (auto& bus_cert : *bus_vec) {
                    Json::Value asset_data = json_helper.prop_list(bus_cert.get_cert());
                    json_value.append(asset_data);
                }
            }
            else {
                auto result_vec = blockchain.get_issued_asset_certs("", cert_type);
                std::sort(result_vec->begin(), result_vec->end());
                for (auto& elem : *result_vec) {
                    Json::Value asset_data = json_helper.prop_list(elem);
                    json_value.append(asset_data);
                }
            }
        }
        else { // list asset certs owned by account
            blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
            auto pvaddr = blockchain.get_account_addresses(auth_.name);
            if (!pvaddr)
                throw address_list_nullptr_exception{"nullptr for address list"};

            auto sh_vec = std::make_shared<chain::asset_cert::list>();
            for (auto& each : *pvaddr) {
                sync_fetch_asset_cert_balance(each.get_address(), "", blockchain, sh_vec, cert_type);
            }

            std::sort(sh_vec->begin(), sh_vec->end());
            for (auto& elem: *sh_vec) {
                Json::Value asset_cert = json_helper.prop_list(elem);
                json_value.append(asset_cert);
            }
        }
    }
    else {
        json_key = "assets";

        if (auth_.name.empty()) { // no account -- list whole assets in blockchain
            auto sh_vec = blockchain.get_issued_assets();
            std::sort(sh_vec->begin(), sh_vec->end());
            for (auto& elem: *sh_vec) {
                Json::Value asset_data = json_helper.prop_list(elem, true);
                asset_data["status"] = "issued";
                json_value.append(asset_data);
            }
        }
        else { // list asset owned by account
            blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
            auto pvaddr = blockchain.get_account_addresses(auth_.name);
            if (!pvaddr)
                throw address_list_nullptr_exception{"nullptr for address list"};

            auto sh_vec = std::make_shared<chain::asset_balances::list>();

            // 1. get asset in blockchain
            // get address unspent asset balance
            for (auto& each : *pvaddr) {
                sync_fetch_asset_balance(each.get_address(), true, blockchain, sh_vec);
            }

            std::sort(sh_vec->begin(), sh_vec->end());
            for (auto& elem: *sh_vec) {
                auto issued_asset = blockchain.get_issued_asset(elem.symbol);
                if (!issued_asset) {
                    continue;
                }
                Json::Value asset_data = json_helper.prop_list(elem, *issued_asset, false);
                asset_data["status"] = "unspent";
                json_value.append(asset_data);
            }

            // 2. get asset in local database
            // shoudl filter all issued asset which be stored in local account asset database
            sh_vec->clear();
            auto sh_unissued = blockchain.get_account_unissued_assets(auth_.name);
            for (auto& elem: *sh_unissued) {
                Json::Value asset_data = json_helper.prop_list(elem.detail, false, false);
                asset_data["status"] = "unissued";
                json_value.append(asset_data);
            }
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
