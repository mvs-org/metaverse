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

#include <jsoncpp/json/json.h>
#include <metaverse/explorer/extensions/commands/validateaddress.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using payment_address = wallet::payment_address;

/************************ validateaddress *************************/

console_result validateaddress::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    std::string version_info;
    std::string message{"valid address"};
    bool is_valid{true};

    auto& blockchain = node.chain_impl();
    const bool use_testnet_rules{blockchain.chain_settings().use_testnet_rules};
    wallet::payment_address payment_address(argument_.address);

    if (blockchain.is_blackhole_address(argument_.address)) {
        version_info = "p2blackhole";
    } else if (payment_address.version() == payment_address::mainnet_p2kh ) {
        if (use_testnet_rules) {
            version_info = "p2kh(test-net)";
        } else {
            version_info = "p2kh(main-net)";
        }
    } else if (payment_address.version() == payment_address::mainnet_p2sh ) {
        version_info = "p2sh(multi-signature)";
    } else {
        version_info = "none";
        message = "invalid address!";
        is_valid = false;
    }

    auto& jv = jv_output;
    if (get_api_version() <= 2) {
        jv["address-type"] = version_info;
        jv["test-net"] = use_testnet_rules;
        jv["is-valid"] = is_valid;
        jv["message"] = message;
    }
    else {
        jv["address_type"] = version_info;
        jv["testnet"] = use_testnet_rules;
        jv["is_valid"] = is_valid;
        jv["message"] = message;
    }

    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

