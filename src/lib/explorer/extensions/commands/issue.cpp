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

    if (argument_.fee < 1000000000)
        throw asset_issue_poundage_exception{"issue asset fee less than 1000000000!"};
    if (argument_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw asset_symbol_length_exception{"asset symbol length must be less than 64."};
    // fail if asset is already in blockchain
    if (blockchain.is_asset_exist(argument_.symbol, false))
        throw asset_symbol_existed_exception{"asset symbol is already exist in blockchain"};
    // local database asset check
    auto sh_asset = blockchain.get_account_unissued_asset(auth_.name, argument_.symbol);
    if (!sh_asset)
        throw asset_symbol_notfound_exception{argument_.symbol + " not found"};

    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if (!pvaddr || pvaddr->empty())
        throw address_list_nullptr_exception{"nullptr for address list"};

    // get random address
    auto addr = get_random_payment_address(pvaddr, blockchain);

    std::string cert_address;
    std::string cert_symbol;
    asset_cert_type cert_type = asset_cert_ns::none;

    // domain cert check
    auto&& domain = asset_cert::get_domain(argument_.symbol);
    if (asset_cert::is_valid_domain(domain)) {
        if (!blockchain.is_asset_cert_exist(domain, asset_cert_ns::domain)) {
            // domain cert does not exist, issue new domain cert to this address
            cert_address = addr;
            cert_type = asset_cert_ns::domain;
            cert_symbol = domain;
        }
        else {
            const auto match = [](const business_address_asset_cert& item, asset_cert_type cert_type) {
                return asset_cert::test_certs(item.certs.get_certs(), cert_type);
            };

            // if domain cert exists then check whether it belongs to the account.
            auto certs_vec = blockchain.get_account_asset_certs(auth_.name, domain);
            auto it = std::find_if(certs_vec->begin(), certs_vec->end(),
                std::bind(match, _1, asset_cert_ns::domain));
            if (it != certs_vec->end()) {
                cert_address = it->address;
                cert_type = asset_cert_ns::domain;
                cert_symbol = domain;
            }
            else {
                // if domain cert does not belong to the account then check domain naming cert
                certs_vec = blockchain.get_account_asset_certs(auth_.name, argument_.symbol);
                it = std::find_if(certs_vec->begin(), certs_vec->end(),
                    std::bind(match, _1, asset_cert_ns::naming));
                if (it != certs_vec->end()) {
                    cert_address = it->address;
                    cert_type = asset_cert_ns::naming;
                    cert_symbol = argument_.symbol;
                }
                else {
                    throw asset_cert_domain_exception{
                        "Domain cert " + domain + " exists in blockchain and does not belong to " + auth_.name};
                }
            }
        }
    }

    // receiver
    std::vector<receiver_record> receiver{
        {addr, argument_.symbol, 0, 0, utxo_attach_type::asset_issue, attachment()}
    };

    // asset_cert utxo
    auto certs = sh_asset->get_asset_cert_mask();
    if (certs != asset_cert_ns::none) {
        receiver.push_back({addr, argument_.symbol, 0, 0,
            certs, utxo_attach_type::asset_cert, attachment()});
    }

    // domain cert or domain naming cert
    if (asset_cert::is_valid_domain(domain)) {
        receiver.push_back({cert_address, cert_symbol, 0, 0,
            cert_type, utxo_attach_type::asset_cert, attachment()});
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

