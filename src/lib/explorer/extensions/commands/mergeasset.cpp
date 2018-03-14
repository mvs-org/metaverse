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
#include <metaverse/explorer/extensions/commands/mergeasset.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


console_result mergeasset::invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(argument_.symbol);

    if (argument_.symbol.empty())
        throw asset_symbol_length_exception{"asset symbol must be non-empty."};

    if (argument_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw asset_symbol_length_exception{"asset symbol length must be less than 64."};

    if (!blockchain.is_valid_address(argument_.address))
        throw toaddress_invalid_exception{"invalid target address parameter! " + argument_.address};

    if (!argument_.mychange_address.empty() && !blockchain.is_valid_address(argument_.mychange_address))
        throw address_invalid_exception{"invalid mychange address! " + argument_.mychange_address};

    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if (!pvaddr)
        throw address_list_nullptr_exception{"empty address list"};

    if (!blockchain.get_account_address(auth_.name, argument_.address))
        throw address_dismatch_account_exception{"address does not match account. " + argument_.address};

    if (!blockchain.get_account_address(auth_.name, argument_.mychange_address))
        throw address_dismatch_account_exception{"address does not match account." + argument_.mychange_address};

    // receiver
    std::vector<receiver_record> receiver{
        {argument_.address, argument_.symbol, 0, 0, utxo_attach_type::asset_transfer, attachment()}
    };
    auto merge_helper = merging_asset(*this, blockchain,
            std::move(auth_.name), std::move(auth_.auth),
            std::move(argument_.symbol), std::move(receiver),
            std::move(argument_.mychange_address),
            argument_.fee);

    merge_helper.exec();

    // json output
    auto tx = merge_helper.get_transaction();
    jv_output =  config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

