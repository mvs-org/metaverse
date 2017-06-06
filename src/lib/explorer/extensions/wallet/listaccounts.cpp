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

#include <boost/property_tree/ptree.hpp>      
#include <boost/property_tree/json_parser.hpp>

#include <metaverse/bitcoin.hpp>
#include <metaverse/client.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/callback_state.hpp>
#include <metaverse/explorer/display.hpp>
#include <metaverse/explorer/prop_tree.hpp>
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/wallet/listaccounts.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."

/************************ listaccounts *************************/

console_result listaccounts::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if(!blockchain.is_admin_account(auth_.name))
        throw std::logic_error{"you are not admin account!"};
    
    auto sh_vec = blockchain.get_accounts();
    
    pt::ptree root;
    pt::ptree accounts;
    const auto action = [&](account& elem)
    {
        pt::ptree acc;
        acc.put("name", elem.get_name());
        acc.put("mnemonic-key", elem.get_mnemonic());
        acc.put("address-count", elem.get_hd_index());
        acc.put("user-status", elem.get_user_status());
        //root.add_child(elem.get_name(), acc);
        accounts.push_back(std::make_pair("", acc));
    };
    std::for_each(sh_vec->begin(), sh_vec->end(), action);
    root.add_child("accounts", accounts);
    pt::write_json(output, root);
    
    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

