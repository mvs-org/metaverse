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
#include <metaverse/explorer/extensions/commands/transferissueright.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


console_result transferissueright::invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    blockchain.uppercase_symbol(argument_.symbol);

    // check asset symbol
    if (argument_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw asset_symbol_length_exception{"asset symbol length must be less than 64."};

    auto sh_asset = blockchain.get_issued_asset(argument_.symbol);
    if (!sh_asset)
        throw asset_symbol_notfound_exception{argument_.symbol + " asset not found"};

    // check from address
    if (!blockchain.is_valid_address(argument_.from))
        throw address_invalid_exception{"invalid from address! " + argument_.from};

    if (!blockchain.get_account_address(auth_.name, argument_.from))
        throw address_dismatch_account_exception{"from address does not match account. " + argument_.from};

    // check target address
    if (!blockchain.is_valid_address(argument_.to))
        throw address_invalid_exception{"invalid address parameter! " + argument_.to};

    std::string cert_owner = asset_cert::get_owner_from_address(argument_.to, blockchain);
    if (cert_owner.empty())
        throw did_address_needed_exception("target address is not an did address. " + argument_.to);

    // check issue right
    auto certs_send = asset_cert_ns::issue;
    auto certs_owned = blockchain.get_address_asset_cert_type(argument_.from, argument_.symbol);
    if (!asset_cert::test_certs(certs_owned, certs_send))
        throw asset_cert_exception("no issue right to transfer");

    // receiver
    std::vector<receiver_record> receiver{
        {argument_.to, argument_.symbol, 0, 0, certs_send, utxo_attach_type::asset_cert, attachment()}
    };

    auto helper = transferring_asset_cert(*this, blockchain, std::move(auth_.name), std::move(auth_.auth),
            std::move(argument_.from), std::move(argument_.symbol), std::move(receiver), argument_.fee);

    helper.exec();

    // json output
    auto tx = helper.get_transaction();
    jv_output =  config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

