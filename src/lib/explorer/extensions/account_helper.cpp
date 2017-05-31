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

#include <metaverse/explorer/extensions/account_helper.hpp>
#include <metaverse/explorer/prop_tree.hpp>
#include <metaverse/explorer/dispatch.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

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
    //const char* cmds[256]{"mnemonic-to-seed", "-l", lang.str().c_str()};
    const char* cmds[64]{0x00};
    int i = 0;
    cmds[i++] = "mnemonic-to-seed";
    cmds[i++] = "-l";
    auto s_lang = lang.str();
    cmds[i++] = s_lang.c_str();
    // words size check
    std::vector<std::string> tokens;
    if(argument_.words.size() == 24) {
	    for(auto& word : argument_.words){
	        cmds[i++] = word.c_str();
	    }
	} else if(argument_.words.size() == 1) { // all words include in ""
		tokens = split(argument_.words.at(0));
	    for(auto& word : tokens){
	        cmds[i++] = word.c_str();
	    }
	} else {
		throw std::logic_error{"words count should be 24, not " + std::to_string(argument_.words.size())};
	}

    if( console_result::okay != dispatch_command(i, cmds , sin, sout, sout)) {
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
    const char* cmds2[]{"getnewaddress", auth_.name.c_str(), option_.passwd.c_str()};
    pt::ptree addresses;
    
    for( idx = 0; idx < option_.hd_index; idx++ ) {
        pt::ptree addr;
        sin.str("");
        sout.str("");
        dispatch_command(3, cmds2 , sin, sout, sout, blockchain);
        addr.put("", sout.str());
        addresses.push_back(std::make_pair("", addr));
    }

    root.add_child("addresses", addresses);
    
    pt::write_json(output, root);
    
    return console_result::okay;
}

/************************ changepasswd *************************/

console_result changepasswd::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    if (acc) {
        blockchain.set_account_passwd(auth_.name, option_.passwd);
    }else{
        throw std::logic_error{"account not found"};
    }

    return console_result::okay;
}

} // libbitcoin
} // explorer
} // commands
