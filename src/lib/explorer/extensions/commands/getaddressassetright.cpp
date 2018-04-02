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
#include <metaverse/explorer/extensions/commands/getaddressassetright.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ getaddressassetright *************************/

console_result getaddressassetright::invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    if(!blockchain.is_valid_address(argument_.address))
        throw address_invalid_exception{"invalid address!"};

    Json::Value assetright;

    // get asset certs
    auto sp_asset_certs = blockchain.get_address_asset_certs(argument_.address, "");
    if (sp_asset_certs) {
        for (const auto& business_cert : *sp_asset_certs) {
            auto cert_type = business_cert.certs.get_certs();
            if (cert_type != asset_cert_ns::none) {
                Json::Value asset_cert;
                asset_cert["address"] = business_cert.address;
                asset_cert["symbol"] = business_cert.certs.get_symbol();
                asset_cert["owner"] = business_cert.certs.get_owner();
                asset_cert["certs"] = asset_cert::get_certs_name(cert_type);
                assetright.append(asset_cert);
            }
        }
    }

    if (get_api_version() == 1 && assetright.isNull()) { //compatible for v1
        jv_output["assetright"] = "";
    } else {
        jv_output["assetright"] = assetright;
    }

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

