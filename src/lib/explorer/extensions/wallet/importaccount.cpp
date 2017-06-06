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
#include <metaverse/explorer/extensions/wallet/importaccount.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."
#if 0
/************************ importaccount *************************/

console_result importaccount::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    std::istringstream sin("");
    std::ostringstream lang("");
    std::ostringstream sout("");
    
    // parameter account name check
    if (blockchain.is_account_exist(auth_.name))
        throw std::logic_error{"account already exist"};

    if (argument_.words.size() > 24)
        throw std::logic_error{"word count must be less than or equal 24"};
    
    for(auto& i : argument_.words){
        sout<<i<<" ";
    }
    
    auto mnemonic = sout.str().substr(0, sout.str().size()-1); // remove last space
    
    // 1. parameter mnemonic check
    lang<<option_.language;
    sin.str("");
    sout.str("");
    //const char* wallet[256]{"mnemonic-to-seed", "-l", lang.str().c_str()};
    const char* wallet[64]{0x00};
    int i = 0;
    wallet[i++] = "mnemonic-to-seed";
    wallet[i++] = "-l";
    auto s_lang = lang.str();
    wallet[i++] = s_lang.c_str();
    // 
    for(auto& word : argument_.words){
        wallet[i++] = word.c_str();
    }

    if( console_result::okay != dispatch_command(i, wallet , sin, sout, sout)) {
        output<<sout.str();
        return console_result::failure;
    }
    
    // 2. check mnemonic exist in account database
    #if 0 // mnemonic is encrypted by passwd so no check now
    auto is_mnemonic_exist = false;
    auto sh_vec = blockchain.get_accounts();
    for(auto& each : *sh_vec) {
        if(mnemonic == each.get_mnemonic()){
            is_mnemonic_exist = true;
            break;
        }
    }
    if(is_mnemonic_exist)
        throw std::logic_error{"mnemonic already exist!"};
	#endif

    // create account
    auto acc = std::make_shared<bc::chain::account>();
    acc->set_name(auth_.name);
    acc->set_passwd(option_.passwd);
    acc->set_mnemonic(mnemonic, option_.passwd);
    //acc->set_hd_index(option_.hd_index); // hd_index updated in getnewaddress
    // flush to db
    blockchain.store_account(acc);

    // generate all account address
    pt::ptree root;
    root.put("name", auth_.name);
    root.put("mnemonic", mnemonic);
    root.put("hd_index", option_.hd_index);
    
    uint32_t idx = 0;
    const char* wallet2[]{"getnewaddress", auth_.name.c_str(), option_.passwd.c_str()};
    pt::ptree addresses;
    
    for( idx = 0; idx < option_.hd_index; idx++ ) {
        pt::ptree addr;
        sin.str("");
        sout.str("");
        dispatch_command(3, wallet2 , sin, sout, sout, blockchain);
        addr.put("", sout.str());
        addresses.push_back(std::make_pair("", addr));
    }

    root.add_child("addresses", addresses);
    
    pt::write_json(output, root);
    
    return console_result::okay;
}
#endif

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

