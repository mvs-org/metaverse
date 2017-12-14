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
#include <metaverse/explorer/extensions/commands/getnewaccount.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/commands/offline_commands_impl.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

/************************ getnewaccount *************************/

console_result getnewaccount::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{

#ifdef NDEBUG
    if (auth_.name.length() > 128 || auth_.name.length() < 3 ||
        auth_.auth.length() > 128 || auth_.auth.length() < 6)
        throw argument_legality_exception{"name length in [3, 128], password length in [6, 128]"};
#endif

    auto& blockchain = node.chain_impl();
    if (blockchain.is_account_exist(auth_.name)){
        throw account_existed_exception{"account already exist"};
    }

    Json::Value root;

    auto acc = std::make_shared<bc::chain::account>();
    acc->set_name(auth_.name);
    acc->set_passwd(auth_.auth);

    bc::explorer::config::language opt_language(option_.language);
    auto&& seed = get_seed();
    auto&& words_list = get_mnemonic_new(opt_language , seed);
    auto&& words = bc::join(words_list);

    root.put("mnemonic", words);
    acc->set_mnemonic(words, auth_.auth);
    
    // flush to db
    auto ret = blockchain.store_account(acc);

    // get 1 new sub-address by default
    std::stringstream sout("");
    std::istringstream sin;
    const char* cmds2[]{"getnewaddress", auth_.name.c_str(), auth_.auth.c_str()};
    sin.str("");
    sout.str("");
    if (dispatch_command(3, cmds2, sin, sout, sout, node) != console_result::okay) {
        throw address_generate_exception(sout.str());
    }
     
    relay_exception(sout);

    root.put("default-address", sout.str());
    
    pt::write_json(output, root);
    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

