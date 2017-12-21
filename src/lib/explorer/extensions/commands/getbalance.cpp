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
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/getbalance.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ getbalance *************************/

console_result getbalance::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    Json::Value aroot;

    auto vaddr = blockchain.get_account_addresses(auth_.name);
    if(!vaddr) throw address_list_nullptr_exception{"nullptr for address list"};

    uint64_t total_confirmed = 0;
    uint64_t total_received = 0;
    uint64_t total_unspent = 0;
    uint64_t total_frozen = 0;

    //balances addr_balance;
    std::string type("all");

    for (auto& i: *vaddr) {
        balances addr_balance{0, 0, 0, 0};
        auto waddr = wallet::payment_address(i.get_address());
        sync_fetchbalance(waddr, type, blockchain, addr_balance, 0);

        total_confirmed += addr_balance.confirmed_balance;
        total_received += addr_balance.total_received;
        total_unspent += addr_balance.unspent_balance;
        total_frozen += addr_balance.frozen_balance;
    }
    
    if (get_api_version() == 1){
        aroot["total-confirmed"] += total_confirmed;
        aroot["total-received"] += total_received;
        aroot["total-unspent"] += total_unspent;
        aroot["total-available"] += (total_unspent - total_frozen);
        aroot["total-frozen"] += total_frozen;
    } else {
        aroot["total-confirmed"] = total_confirmed;
        aroot["total-received"] = total_received;
        aroot["total-unspent"] = total_unspent;
        aroot["total-available"] = (total_unspent - total_frozen);
        aroot["total-frozen"] = total_frozen;
    }
    output << aroot.toStyledString();

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

