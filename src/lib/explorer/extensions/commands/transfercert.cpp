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
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    blockchain.uppercase_symbol(argument_.symbol);
    boost::to_lower(argument_.cert);

    // check asset symbol
    check_asset_symbol(argument_.symbol);

    // check asset cert types
    auto& cert_type_name = argument_.cert;
    auto match = [&cert_type_name](const std::pair<asset_cert_type, std::string> & item) {
        return item.second == cert_type_name;
    };
    const auto& type_name_map = asset_cert::get_type_name_map();
    auto iter = std::find_if(type_name_map.begin(), type_name_map.end(), match);
    if (iter == type_name_map.end()) {
        throw asset_cert_exception("unsupported asset cert type " + cert_type_name);
    }

    auto cert_type = iter->first;
    if (cert_type == asset_cert_ns::issue) {
        auto sh_asset = blockchain.get_issued_asset(argument_.symbol);
        if (!sh_asset)
            throw asset_symbol_notfound_exception(
                "asset '" + argument_.symbol + "' does not exist.");
    }

    // check target address
    auto to_did = argument_.to;
    auto to_address = get_address_from_did(to_did, blockchain);
    if (!blockchain.is_valid_address(to_address))
        throw toaddress_invalid_exception{"invalid did parameter! " + to_did};

    // check cert is owned by the account
    std::shared_ptr<asset_cert> cert = blockchain.get_asset_cert(argument_.symbol, cert_type);
    if (!cert) {
        throw asset_cert_notfound_exception(
            cert_type_name + " cert '" + argument_.symbol + "' does not exist.");
    }

    auto from_did = cert->get_owner();
    auto from_address = cert->get_address();
    if (!blockchain.get_account_address(auth_.name, from_address)) {
        throw asset_cert_notowned_exception(
            cert_type_name + " cert '" + argument_.symbol + "' is not owned by " + auth_.name);
    }

    account_multisig acc_multisig;
    bool is_multisig_address = blockchain.is_script_address(from_address);
    if (is_multisig_address) {
        auto multisig_vec = acc->get_multisig(from_address);
        if (!multisig_vec || multisig_vec->empty()) {
            throw multisig_notfound_exception{"from address multisig record not found."};
        }

        acc_multisig = *(multisig_vec->begin());
    }

    // receiver
    std::vector<receiver_record> receiver{
        {to_address, argument_.symbol, 0, 0,
            cert_type, utxo_attach_type::asset_cert_transfer, attachment(from_did, to_did)}
    };

    auto helper = transferring_asset_cert(*this, blockchain,
        std::move(auth_.name), std::move(auth_.auth),
        is_multisig_address ? std::move(from_address) : "",
        std::move(argument_.symbol),
        std::move(receiver), argument_.fee,
        std::move(acc_multisig));

    helper.exec();

        // json output
    if (is_multisig_address) {
        // out rawtx for multisig cert
        auto && tx = helper.get_transaction();
        std::ostringstream tx_buf;
        tx_buf << config::transaction(tx);
        jv_output = tx_buf.str();
    }
    else {
        auto tx = helper.get_transaction();
        jv_output =  config::json_helper(get_api_version()).prop_tree(tx, true);
    }

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

