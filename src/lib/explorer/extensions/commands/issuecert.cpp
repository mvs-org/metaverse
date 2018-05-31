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
#include <metaverse/explorer/extensions/commands/issuecert.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


console_result issuecert::invoke (Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    blockchain.uppercase_symbol(argument_.symbol);
    boost::to_lower(argument_.cert);

    // check asset symbol
    check_asset_symbol(argument_.symbol);

    auto to_did = argument_.to;
    auto to_address = get_address_from_did(to_did, blockchain);
    if (!blockchain.is_valid_address(to_address))
        throw address_invalid_exception{"invalid did parameter! " + to_did};

    // check asset cert types
    std::map <std::string, asset_cert_type> cert_map = {
        {"naming", asset_cert_ns::naming}
    };
    auto iter = cert_map.find(argument_.cert);
    if (iter == cert_map.end()) {
        throw asset_cert_exception("unknown asset cert type " + argument_.cert);
    }
    auto certs_create = iter->second;

    std::string domain_cert_addr;
    std::string domain_cert_did;
    if (certs_create == asset_cert_ns::naming) {
        // check symbol is valid.
        auto pos = argument_.symbol.find(".");
        if (pos == std::string::npos) {
            throw asset_symbol_name_exception("invalid naming cert symbol " + argument_.symbol
                + ", it should contain a dot '.'");
        }

        auto&& domain = asset_cert::get_domain(argument_.symbol);
        if (!asset_cert::is_valid_domain(domain)) {
            throw asset_symbol_name_exception("invalid naming cert symbol " + argument_.symbol
                + ", it should contain a valid domain!");
        }

        // check domain naming cert not exist.
        if (blockchain.is_asset_cert_exist(argument_.symbol, asset_cert_ns::naming)) {
            throw asset_cert_existed_exception(
                "naming cert '" + argument_.symbol + "' already exists on the blockchain!");
        }

        // check asset not exist.
        if (blockchain.is_asset_exist(argument_.symbol, false)) {
            throw asset_symbol_existed_exception(
                "asset symbol '" + argument_.symbol + "' already exists on the blockchain!");
        }

        // check domain cert belong to this account.
        bool exist = blockchain.is_asset_cert_exist(domain, asset_cert_ns::domain);
        if (!exist) {
            throw asset_cert_notfound_exception("no domain cert '" + domain + "' found!");
        }

        auto cert = blockchain.get_account_asset_cert(auth_.name, domain, asset_cert_ns::domain);
        if (!cert) {
            throw asset_cert_notowned_exception("no domain cert '" + domain + "' owned by " + auth_.name);
        }

        domain_cert_did = cert->get_owner();
        domain_cert_addr = get_address_from_did(domain_cert_did, blockchain);
    }

    // receiver
    std::vector<receiver_record> receiver{
        {to_address, argument_.symbol, 0, 0,
            certs_create, utxo_attach_type::asset_cert_issue,
            attachment(domain_cert_did, to_did)}
    };

    if (certs_create == asset_cert_ns::naming) {
        auto&& domain = asset_cert::get_domain(argument_.symbol);
        receiver.push_back(
            {domain_cert_addr, domain, 0, 0,
                asset_cert_ns::domain, utxo_attach_type::asset_cert,
                attachment(domain_cert_did, domain_cert_did)}
        );
    }

    auto helper = issuing_asset_cert(*this, blockchain,
        std::move(auth_.name), std::move(auth_.auth),
        "", std::move(argument_.symbol),
        std::move(receiver), argument_.fee);

    helper.exec();

    // json output
    auto tx = helper.get_transaction();
    jv_output = config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

