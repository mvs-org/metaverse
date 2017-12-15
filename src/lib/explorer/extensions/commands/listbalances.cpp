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


#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/listbalances.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ listbalances *************************/

console_result listbalances::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    Json::Value aroot;
    Json::Value all_balances;
    Json::Value address_balances;
    
    auto vaddr = blockchain.get_account_addresses(auth_.name);
    if(!vaddr) throw address_list_nullptr_exception{"nullptr for address list"};

    std::string type("all");
    
    for (auto& i: *vaddr){
        Json::Value address_balance;
        balances addr_balance{0, 0, 0, 0};
        auto waddr = wallet::payment_address(i.get_address());
        sync_fetchbalance(waddr, type, blockchain, addr_balance, 0);
        address_balance["address"] = i.get_address();
        address_balance["confirmed"] = +(addr_balance.confirmed_balance);
        address_balance["received"] = +addr_balance.total_received;
        address_balance["unspent"] = +addr_balance.unspent_balance;
        address_balance["available"] = +addr_balance.unspent_balance - addr_balance.frozen_balance;
        address_balance["frozen"] = +addr_balance.frozen_balance;
         
        Json::Value null_balances;
        // non_zero display options
        if (option_.non_zero){
            if (addr_balance.unspent_balance){
                null_balances["balance"] = address_balance;
                all_balances.append(null_balances);
            }
        } else {
            null_balances["balance"] = address_balance;
            all_balances.append(null_balances);
        }
    }
    
    aroot["balances"] = all_balances;
    output << aroot.toStyledString();
    return console_result::okay;

}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

