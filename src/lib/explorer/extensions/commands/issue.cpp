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
#include <metaverse/explorer/extensions/commands/issue.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>
#include <metaverse/bitcoin/chain/attachment/asset/asset_detail.hpp>

using std::placeholders::_1;

namespace libbitcoin {
namespace explorer {
namespace commands {


console_result issue::invoke (Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();

    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(argument_.symbol);

    // check asset symbol
    check_asset_symbol(argument_.symbol);

    if (argument_.fee < 1000000000)
        throw asset_issue_poundage_exception{"issue asset fee less than 1000000000!"};
    // fail if asset is already in blockchain
    if (blockchain.is_asset_exist(argument_.symbol, false))
        throw asset_symbol_existed_exception{"asset symbol is already exist in blockchain"};
    // local database asset check
    auto sh_asset = blockchain.get_account_unissued_asset(auth_.name, argument_.symbol);
    if (!sh_asset)
        throw asset_symbol_notfound_exception{argument_.symbol + " not found"};

    auto to_did = sh_asset->get_issuer();
    auto to_address = get_address_from_did(to_did, blockchain);
    if (!blockchain.is_valid_address(to_address)) {
        throw address_invalid_exception{"invalid asset issuer " + to_did};
    }

    std::string cert_address;
    std::string cert_symbol;
    std::string cert_did;
    asset_cert_type cert_type = asset_cert_ns::none;
    bool is_domain_cert_exist = false;

    // domain cert check
    auto&& domain = asset_cert::get_domain(argument_.symbol);
    if (asset_cert::is_valid_domain(domain)) {
        bool exist = blockchain.is_asset_cert_exist(domain, asset_cert_ns::domain);
        if (!exist) {
            // domain cert does not exist, issue new domain cert to this address
            is_domain_cert_exist = false;
            cert_address = to_address;
            cert_type = asset_cert_ns::domain;
            cert_symbol = domain;
            cert_did = to_did;
        }
        else {
            // if domain cert exists then check whether it belongs to the account.
            is_domain_cert_exist = true;
            auto cert = blockchain.get_account_asset_cert(auth_.name, domain, asset_cert_ns::domain);
            if (cert) {
                cert_symbol = domain;
                cert_type = cert->get_type();
                cert_did = cert->get_owner();
                cert_address = get_address_from_did(cert_did, blockchain);
            }
            else {
                // if domain cert does not belong to the account then check naming cert
                exist = blockchain.is_asset_cert_exist(argument_.symbol, asset_cert_ns::naming);
                if (!exist) {
                    throw asset_cert_notfound_exception{
                        "Domain cert " + argument_.symbol + " exists on the blockchain and is not owned by " + auth_.name};
                }
                else {
                    cert = blockchain.get_account_asset_cert(auth_.name, argument_.symbol, asset_cert_ns::naming);
                    if (!cert) {
                        throw asset_cert_notowned_exception{
                            "No domain cert or naming cert owned by " + auth_.name};
                    }

                    cert_symbol = argument_.symbol;
                    cert_type = cert->get_type();
                    cert_did = cert->get_owner();
                    cert_address = get_address_from_did(cert_did, blockchain);
                }
            }
        }
    }

    // receiver
    std::vector<receiver_record> receiver{
        {to_address, argument_.symbol, 0, 0, utxo_attach_type::asset_issue, attachment(to_did, to_did)}
    };

    // asset_cert utxo
    auto certs = sh_asset->get_asset_cert_mask();
    if (!certs.empty()) {
        for (auto each_cert_type : certs) {
            receiver.push_back({to_address, argument_.symbol, 0, 0,
                each_cert_type, utxo_attach_type::asset_cert_autoissue, attachment(to_did, to_did)});
        }
    }

    // domain cert or naming cert
    if (asset_cert::is_valid_domain(domain)) {
        receiver.push_back({cert_address, cert_symbol, 0, 0, cert_type,
            (is_domain_cert_exist ? utxo_attach_type::asset_cert : utxo_attach_type::asset_cert_autoissue),
            attachment(cert_did, cert_did)});
    }

    auto issue_helper = issuing_asset(*this, blockchain,
        std::move(auth_.name), std::move(auth_.auth),
        "", std::move(argument_.symbol),
        std::move(option_.attenuation_model_param),
        std::move(receiver), argument_.fee);

    issue_helper.exec();

    // json output
    auto tx = issue_helper.get_transaction();
    jv_output =  config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

