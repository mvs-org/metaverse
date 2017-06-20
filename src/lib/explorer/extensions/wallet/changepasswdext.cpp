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
#include <metaverse/explorer/extensions/wallet/changepasswdext.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."
/************************ changepasswdext *************************/

console_result changepasswdext::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    std::istringstream sin("");
    std::ostringstream lang("");
    std::ostringstream sout("");
    
    // parameter check    
    auto user = std::make_shared<bc::chain::account>();

    for(auto& i : argument_.words){
        sout<<i<<" ";
    }
    
    auto mnemonic = sout.str().substr(0, sout.str().size()-1); // remove last space
    auto& blockchain = node.chain_impl();
    if(!auth_.name.empty()) {
        auto acc = blockchain.get_account(auth_.name);
        if(!acc)
            throw std::logic_error{"non exist account name"};
        
        if(mnemonic != acc->get_mnemonic())
            throw std::logic_error{"account is not exist for this mnemonic!"};
        
        user = acc;
    } else { // scan all account to compare mnemonic
        auto sh_vec = blockchain.get_accounts();
        for(auto& each : *sh_vec) {
            if(mnemonic == each.get_mnemonic()){
                *user = each;
                break;
            }
        }
    }

    if(mnemonic != user->get_mnemonic())
        throw std::logic_error{"account is not exist for this mnemonic!"};

    lang<<option_.language;
    sin.str("");
    sout.str("");
    //const char* wallet[256]{"mnemonic-to-seed", "-l", lang.str().c_str()};
    const char* wallet[256]{0x00};
    int i = 0;
    wallet[i++] = "mnemonic-to-seed";
    wallet[i++] = "-l";
    wallet[i++] = lang.str().c_str();
    for(auto& word : argument_.words){
        wallet[i++] = word.c_str();
    }

    if( console_result::okay != dispatch_command(i, wallet , sin, sout, sout)) {
        output<<sout.str();
        return console_result::failure;
    }

    user->set_passwd(option_.passwd);
    
    // flush to db
    blockchain.store_account(user);

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

