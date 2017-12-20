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
#include <metaverse/explorer/extensions/commands/deleteaccount.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ deleteaccount *************************/

console_result deleteaccount::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    std::string mnemonic;
    acc->get_mnemonic(auth_.auth, mnemonic);
    std::vector<std::string> results;
    boost::split(results, mnemonic, boost::is_any_of(" "));

    if (*results.rbegin() != argument_.last_word){
        throw argument_dismatch_exception{"last word not matching."};
    }
    // delete account addresses
    blockchain.delete_account_address(acc->get_name());

    // delete account asset
    blockchain.delete_account_asset(acc->get_name());
    // delete account
    blockchain.delete_account(acc->get_name());

    Json::Value jv;
    jv["name"] = acc->get_name();
    jv["status"]= "removed successfully";

    output<<jv.toStyledString();

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

