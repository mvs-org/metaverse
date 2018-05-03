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

console_result getaddressasset::invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    if(!blockchain.is_valid_address(argument_.address))
        throw address_invalid_exception{"invalid address!"};

    if (option_.is_cert) { // only get asset certs
        Json::Value assetcerts;

        // get asset certs
        auto sp_asset_certs = blockchain.get_address_asset_certs(argument_.address, "");
        if (sp_asset_certs) {
            for (const auto& business_cert : *sp_asset_certs) {
                if (business_cert.certs.get_certs() != asset_cert_ns::none) {
                    Json::Value asset_cert = config::json_helper(get_api_version()).prop_list(business_cert.certs);
                    asset_cert["address"] = business_cert.address;
                    assetcerts.append(asset_cert);
                }
            }
        }

        if (get_api_version() == 1 && assetcerts.isNull()) { //compatible for v1
            jv_output["assetcerts"] = "";
        } else {
            jv_output["assetcerts"] = assetcerts;
        }

        return console_result::okay;
    }

    Json::Value assets;
    auto sh_vec = std::make_shared<asset_balances::list>();
    sync_fetch_asset_balance(argument_.address, true, blockchain, sh_vec);

    for (auto& elem: *sh_vec) {
        auto issued_asset = blockchain.get_issued_asset(elem.symbol);
        if (!issued_asset) {
            continue;
        }
        Json::Value asset_data = config::json_helper(get_api_version()).prop_list(elem, *issued_asset);
        asset_data["status"] = "unspent";
        assets.append(asset_data);
    }

    if (get_api_version() == 1 && assets.isNull()) { //compatible for v1
        jv_output["assets"] = "";
    }
    else {
        jv_output["assets"] = assets;
    }

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

