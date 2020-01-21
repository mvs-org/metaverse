/**
 * Copyright (c) 2016-2020 mvs developers
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
#include <metaverse/explorer/extensions/commands/setminingaccount.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ setminingaccount *************************/

console_result setminingaccount::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto& miner = node.miner();

    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if (!pvaddr)
        throw address_list_nullptr_exception{"nullptr for address list"};

#if 0 // no random address required for miner
    auto pubkey = pvaddr->begin()->get_pub_key();
#else
    auto is_found = blockchain.get_account_address(auth_.name, argument_.payment_address.encoded());
    if (!is_found)
        throw address_dismatch_account_exception{"address does not match account."};
#endif

    auto ret = miner.set_miner_payment_address(argument_.payment_address);
    if (ret) {
        jv_output = "Address [" + argument_.payment_address.encoded() + "] setted.";
    } else {
        throw unknown_error_exception{"set mining account solo mining got an error"};
    }

    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

