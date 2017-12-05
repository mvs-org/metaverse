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
#include <metaverse/explorer/extensions/wallet/getaccount.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."

/************************ getaccount *************************/

console_result getaccount::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    //auto&& mnemonic = acc->get_mnemonic(auth_.auth);
    std::string mnemonic;
    acc->get_mnemonic(auth_.auth, mnemonic);
    std::vector<std::string> results;
    boost::split(results, mnemonic, boost::is_any_of(" "));

    if (*results.rbegin() != argument_.last_word){
        throw argument_dismatch_exception{"last word not matching."};
    }

    pt::ptree root;
    root.put("name", acc->get_name());
    root.put("mnemonic-key", mnemonic);
    root.put("address-count", acc->get_hd_index());
    root.put("user-status", acc->get_user_status());
    pt::write_json(output, root);

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

