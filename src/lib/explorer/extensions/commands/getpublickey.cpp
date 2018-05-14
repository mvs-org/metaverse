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


#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/getpublickey.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

console_result getpublickey::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if (!argument_.address.empty() && !blockchain.is_valid_address(argument_.address))
        throw address_invalid_exception{"invalid address parameter!"};

    auto addr = bc::wallet::payment_address(argument_.address);
    if(addr.version() == bc::wallet::payment_address::mainnet_p2sh) // for multisig address
        throw argument_legality_exception{"script address parameter not allowed!"};

    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr)
        throw address_list_nullptr_exception{"nullptr for address list"};

    // set random address
    if (argument_.address.empty()) {
        argument_.address = get_random_payment_address(pvaddr, blockchain);
    }

    // get public key
    std::string prv_key;
    std::string pub_key;
    auto found = false;
    for (auto& each : *pvaddr){
        if (each.get_address() == argument_.address) {
            prv_key = each.get_prv_key(auth_.auth);
            pub_key = ec_to_xxx_impl("ec-to-public", prv_key);

            found = true;
            break;
        }
    }

    if(!found) {
        throw account_address_get_exception{pub_key};
    }

    auto& root = jv_output;
    root["public-key"] = pub_key;
    root["address"] = argument_.address;

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

