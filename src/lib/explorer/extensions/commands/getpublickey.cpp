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
#include <metaverse/bitcoin.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

console_result getpublickey::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    const auto& arg_address = argument_.address;

    std::string pub_key;
    std::string address;

    data_chunk public_key_data;
    if (!option_.check_did) {
        wallet::ec_private wif_key(arg_address);
        if (wif_key) {
            auto&& ec_public_key = wif_key.to_public();
            ec_public_key.to_data(public_key_data);
        }
        else {
            ec_secret private_key;
            if (decode_base16(private_key, arg_address)
                && bc::verify(private_key)
                && !private_key.empty()) {
                wallet::ec_private ec_private_key(private_key, 0, true);
                auto&& ec_public_key = ec_private_key.to_public();
                ec_public_key.to_data(public_key_data);
            }
            else {
                decode_base16(public_key_data, arg_address);
            }
        }
    }

    if (!option_.check_did && is_public_key(public_key_data)) {
        pub_key = encode_base16(public_key_data);
        auto pay_address = wallet::ec_public(public_key_data).to_payment_address();
        address = pay_address.encoded();
    }
    else {
        blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
        auto pvaddr = blockchain.get_account_addresses(auth_.name);
        if(!pvaddr)
            throw address_list_nullptr_exception{"nullptr for address list"};

        if (!arg_address.empty()) {
            address = get_address(arg_address, blockchain);
        }
        else {
            // set random address
            address = get_random_payment_address(pvaddr, blockchain);
        }

        auto addr = bc::wallet::payment_address(address);
        if(addr.version() == bc::wallet::payment_address::mainnet_p2sh) // for multisig address
            throw argument_legality_exception{"script address parameter not allowed!"};

        // get public key
        std::string prv_key;
        auto found = false;
        for (auto& each : *pvaddr){
            if (each.get_address() == address) {
                prv_key = each.get_prv_key(auth_.auth);
                pub_key = ec_to_xxx_impl("ec-to-public", prv_key);

                found = true;
                break;
            }
        }

        if (!found) {
            throw address_dismatch_account_exception{"target did/address does not match account. " + arg_address};
        }
    }

    auto& root = jv_output;
    if (get_api_version() <= 2) {
        root["public-key"] = pub_key;
        root["address"] = address;
    }
    else {
        root["public_key"] = pub_key;
        root["address"] = address;
    }

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

