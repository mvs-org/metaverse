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
#include <metaverse/explorer/extensions/commands/getnewaccount.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."

/************************ getnewaccount *************************/

console_result getnewaccount::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    if (blockchain.is_account_exist(auth_.name)){
        throw account_existed_exception{"account already exist"};
    }

    const char* cmds[]{"seed"};
    //, "mnemonic-to-seed", "hd-new", 
    //    "hd-to-ec", "ec-to-public", "ec-to-address"};
    std::stringstream sout("");
    std::istringstream sin;
    pt::ptree root;

    auto acc = std::make_shared<bc::chain::account>();

#ifdef NDEBUG
    if (auth_.name.length() > 128 || auth_.name.length() < 3 ||
        auth_.auth.length() > 128 || auth_.auth.length() < 6)
        throw argument_legality_exception{"name length in [3, 128], password length in [6, 128]"};
#endif

    acc->set_name(auth_.name);
    acc->set_passwd(auth_.auth);

    auto exec_with = [&](int i){
        sin.str(sout.str());
        sout.str("");
        return dispatch_command(1, cmds + i, sin, sout, sout);
    };
     
    if (exec_with(0) != console_result::okay) {
        throw seed_exception(sout.str());
    }
     
    relay_exception(sout);

    const char* cmds3[3]{"mnemonic-new", "-l" , option_.language.c_str()};
    sin.str(sout.str());
    sout.str("");
    if (dispatch_command(3, cmds3, sin, sout, sout) != console_result::okay) {
        throw mnemonicwords_new_exception(sout.str());
    }

     
    relay_exception(sout);

    root.put("mnemonic", sout.str());
    acc->set_mnemonic(sout.str(), auth_.auth);
    
    // flush to db
    auto ret = blockchain.store_account(acc);

    // get 1 new sub-address by default
    const char* cmds2[]{"getnewaddress", auth_.name.c_str(), auth_.auth.c_str()};
    sin.str("");
    sout.str("");
    if (dispatch_command(3, cmds2, sin, sout, sout, node) != console_result::okay) {
        throw address_generate_exception(sout.str());
    }
     
    relay_exception(sout);

    #if 0
    // parse address from getnewaddress output string
    pt::ptree tx;
    sin.str(sout.str());
    pt::read_json(sin, tx);
    
    auto addr_array = tx.get_child("addresses");
    
    for(auto& addr : addr_array) 
        sout.str(addr.second.data());
    #endif
    root.put("default-address", sout.str());
    
    pt::write_json(output, root);
    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

