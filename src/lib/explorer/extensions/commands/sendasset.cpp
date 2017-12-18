/**
 * Copyright (c) 2016-2017 mvs developers 
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
#include <metaverse/explorer/extensions/commands/sendasset.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


console_result sendasset::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(argument_.symbol);
    
    if (argument_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw asset_symbol_length_exception{"asset symbol length must be less than 64."};
    if (!blockchain.is_valid_address(argument_.address))
        throw address_invalid_exception{"invalid to address parameter!"};
    if (!argument_.amount)
        throw asset_amount_exception{"invalid asset amount parameter!"};

    // receiver
    std::vector<receiver_record> receiver{
        {argument_.address, argument_.symbol, 0, argument_.amount, utxo_attach_type::asset_transfer, attachment()}  
    };
    auto send_helper = sending_asset(*this, blockchain, std::move(auth_.name), std::move(auth_.auth), 
            "", std::move(argument_.symbol), std::move(receiver), argument_.fee);
#if 0
    auto send_helper = sending_locked_asset(*this, blockchain, std::move(auth_.name), std::move(auth_.auth), 
            "", std::move(argument_.symbol), std::move(receiver), argument_.fee, argument_.lockedtime);
#endif
    
    send_helper.exec();

    // json output
    auto tx = send_helper.get_transaction();
    output << config::json_helper().prop_tree(tx, true).toStyledString();
    log::debug("command")<<"transaction="<<output.rdbuf();

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

