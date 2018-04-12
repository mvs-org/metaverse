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
#include <metaverse/explorer/extensions/commands/issuedid.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp> 
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


console_result issuedid::invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(argument_.symbol);
    if(bc::wallet::symbol::is_sensitive(argument_.symbol)) {
        throw asset_symbol_name_exception{"invalid symbol start with " + argument_.symbol};
    }

    if(argument_.fee < 100000000)
        throw did_issue_poundage_exception{"issue did fee less than 100000000 satoshi = 1 etp!"};
    if (argument_.symbol.length() > DID_DETAIL_SYMBOL_FIX_SIZE)
        throw did_symbol_length_exception{"did symbol length must be less than 64."};
    if (!blockchain.is_valid_address(argument_.address))
        throw address_invalid_exception{"invalid address parameter!"};

    if (!blockchain.get_account_address(auth_.name, argument_.address))
         throw address_dismatch_account_exception{"target address does not match account. " + argument_.address};
        
    // fail if did is already in blockchain
    if(blockchain.is_did_exist(argument_.symbol))
        throw did_symbol_existed_exception{"did symbol is already exist in blockchain"};

    // fail if address is already binded with did in blockchain
    if(blockchain.is_address_issued_did(argument_.address))
        throw did_symbol_existed_exception{"address is already binded with some did in blockchain"};


    // receiver
    std::vector<receiver_record> receiver{
        {argument_.address, argument_.symbol, 0, 0, utxo_attach_type::did_issue, attachment()}  
    };
    auto issue_helper = issuing_did(*this, blockchain, std::move(auth_.name), std::move(auth_.auth), 
            std::move(argument_.address), std::move(argument_.symbol), std::move(receiver), argument_.fee);
    
    issue_helper.exec();
    // json output
    auto tx = issue_helper.get_transaction();

    jv_output =  config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

