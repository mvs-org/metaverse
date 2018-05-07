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
#include <metaverse/explorer/extensions/commands/transfercert.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


console_result transfercert::invoke (Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    blockchain.uppercase_symbol(argument_.symbol);

    // check asset symbol
    if (argument_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw asset_symbol_length_exception{"asset symbol length must be less than 64."};

    // check asset cert types
    std::map <std::string, asset_cert_type> cert_param_value_map = {
        {"ISSUE", asset_cert_ns::issue},
        {"DOMAIN", asset_cert_ns::domain},
        {"NAMING", asset_cert_ns::naming}
    };

    auto certs_send = asset_cert_ns::none;
    for (auto& cert_param : argument_.certs) {
        blockchain.uppercase_symbol(cert_param);
        auto iter = cert_param_value_map.find(cert_param);
        if (iter == cert_param_value_map.end()) {
            throw asset_cert_exception("unknown asset cert type " + cert_param);
        }

        certs_send |= iter->second;
    }

    if (certs_send == asset_cert_ns::none) {
        throw asset_cert_exception("no asset cert type is specified to transfer");
    }

    if (asset_cert::test_certs(certs_send, asset_cert_ns::issue)) {
        auto sh_asset = blockchain.get_issued_asset(argument_.symbol);
        if (!sh_asset)
            throw asset_symbol_notfound_exception{argument_.symbol + " asset not found"};
    }

    // check target address
    auto to_did = argument_.to;
    auto to_address = get_address_from_did(to_did, blockchain);
    if (!blockchain.is_valid_address(to_address))
        throw address_invalid_exception{"invalid did parameter! " + to_did};

    // receiver
    std::vector<receiver_record> receiver{
        {to_address, argument_.symbol, 0, 0,
            certs_send, utxo_attach_type::asset_cert, attachment("", to_did)}
    };

    auto helper = transferring_asset_cert(*this, blockchain,
        std::move(auth_.name), std::move(auth_.auth),
        "", std::move(argument_.symbol),
        std::move(receiver), argument_.fee);

    helper.exec();

    // json output
    auto tx = helper.get_transaction();
    jv_output =  config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

