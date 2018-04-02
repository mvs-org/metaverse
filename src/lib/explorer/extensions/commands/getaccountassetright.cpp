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
#include <metaverse/explorer/extensions/commands/getaccountassetright.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ getaccountassetright *************************/

console_result getaccountassetright::invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    if (argument_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw asset_symbol_length_exception{"asset symbol length must be less than 64."};

    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr)
        throw address_list_nullptr_exception{"nullptr for address list"};

    Json::Value assetright;

    auto sh_vec = std::make_shared<std::vector<asset_detail>>();

    // get asset certs
    auto sp_asset_certs = blockchain.get_account_asset_certs(auth_.name, argument_.symbol);
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

