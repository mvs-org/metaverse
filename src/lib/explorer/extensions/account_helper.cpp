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
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/account/account_info.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

/************************ importaccount *************************/
console_result importaccount::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    std::istringstream sin("");
    std::ostringstream lang("");
    std::stringstream sout("");
    
    // parameter account name check
    auto& blockchain = node.chain_impl();
    if (blockchain.is_account_exist(auth_.name))
     throw account_existed_exception{"account already exist"};

	if (auth_.name.length() > 128 || auth_.name.length() < 3 ||
		option_.passwd.length() > 128 || option_.passwd.length() < 6)
		throw argument_exceed_limit_exception{"name length in [3, 128], password length in [6, 128]"};
    
    if (argument_.words.size() > 24)
        throw argument_exceed_limit_exception{"word count must be less than or equal 24"};
    
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
        throw argument_size_invalid_exception{"words count should be 24, not " + std::to_string(argument_.words.size())};
    }

    if(dispatch_command(i, cmds , sin, sout, sout) != console_result::okay) {
        throw mnemonicwords_to_seed_exception(sout.str());
    }
     
     
    relay_exception(sout);
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
        throw mnemonicwords_existed_exception{"mnemonic already exist!"};
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
        if (dispatch_command(3, cmds2, sin, sout, sout, node) != console_result::okay) {
            throw address_generate_exception(sout.str());
        }
         
        relay_exception(sout);
        addr.put("", sout.str());
        addresses.push_back(std::make_pair("", addr));
    }

    root.add_child("addresses", addresses);
    
    pt::write_json(output, root);
    
    return console_result::okay;
}

/************************ changepasswd *************************/

console_result changepasswd::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    std::string mnemonic;
    acc->get_mnemonic(auth_.auth, mnemonic);
    
    acc->set_passwd(option_.passwd);
    acc->set_mnemonic(mnemonic, option_.passwd);

    blockchain.store_account(acc);
    
    // reencry address
    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr) 
        throw address_list_nullptr_exception{"empty address list"};
    
    std::string prv_key;
    for (auto& each : *pvaddr){
        prv_key = each.get_prv_key(auth_.auth);
        each.set_prv_key(prv_key, option_.passwd);
    }
    // delete all old address
    blockchain.delete_account_address(auth_.name);
    // restore address
    for (auto& each : *pvaddr) {
        auto addr = std::make_shared<bc::chain::account_address>(each);
        blockchain.store_account_address(addr);
    }

    return console_result::okay;
}

/************************ getnewmultisig *************************/

console_result getnewmultisig::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    // parameter account name check
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    //auto acc_multisig = acc->get_multisig();
    account_multisig acc_multisig;
            
    if(option_.public_keys.empty())
        throw multisig_cosigne_exception{"multisig cosigner public key needed."};
    // parameter check
    if( option_.m < 1 )
        throw signature_amount_exception{"signature number less than 1."};
    if( !option_.n || option_.n > 20 )
        throw pubkey_amount_exception{"public key number bigger than 20."};
    if( option_.m > option_.n )
        throw signature_amount_exception{"signature number bigger than public key number."};

    // add self public key into key vector
    auto pubkey = option_.self_publickey;
    if(std::find(option_.public_keys.begin(), option_.public_keys.end(), pubkey) == option_.public_keys.end()) // not found
        option_.public_keys.push_back(pubkey);
    if( option_.n != option_.public_keys.size() )
        throw pubkey_amount_exception{"public key number not match with n."};

    acc_multisig.set_hd_index(0);
    acc_multisig.set_m(option_.m);
    acc_multisig.set_n(option_.n);
    acc_multisig.set_pubkey(pubkey);
    acc_multisig.set_cosigner_pubkeys(std::move(option_.public_keys));
    acc_multisig.set_description(option_.description);
    
    if(acc->get_multisig(acc_multisig))
        throw multisig_exist_exception{"multisig already exists."};
    
    acc_multisig.set_index(acc->get_multisig_vec().size() + 1);
    
    // change account type
    acc->set_type(account_type::multisignature);


    // store address
    auto addr = std::make_shared<bc::chain::account_address>();
    addr->set_name(auth_.name);
    
    // get private key according public key
    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr) 
        throw address_list_nullptr_exception{"nullptr for address list"};
    
    const char* cmds[2]{"ec-to-public", nullptr};
    std::stringstream sout("");
    std::istringstream sin; 
    std::string prv_key;
    auto found = false;
    for (auto& each : *pvaddr){
        prv_key = each.get_prv_key(auth_.auth);
        cmds[1] = prv_key.c_str();
        if(console_result::okay == dispatch_command(2, cmds, sin, sout, sout)) {
            log::trace("pubkey")<<sout.str();
            if(sout.str() == pubkey){
                found = true;
                break;
            }
            sout.str("");
        }
    }
    if(!found)
        throw pubkey_dismatch_exception{pubkey + " not belongs to this account"};

    addr->set_prv_key(prv_key, auth_.auth);

    // multisig address
    auto multisig_script = acc_multisig.get_multisig_script();
    chain::script script_inst;
    script_inst.from_string(multisig_script);
    if(script_pattern::pay_multisig != script_inst.pattern())
        throw multisig_script_exception{std::string("invalid multisig script : ")+multisig_script};
    payment_address address(script_inst, 5);
    
    addr->set_address(address.encoded());
    //addr->set_status(1); // 1 -- enable address
    addr->set_status(account_address_status::multisig_addr);

    auto addr_str = address.encoded();
    acc_multisig.set_address(addr_str);
    acc->set_multisig(acc_multisig);
        
    blockchain.store_account(acc);
    blockchain.store_account_address(addr);
        
    pt::ptree root, pubkeys;

    root.put("index", acc_multisig.get_index());
    root.put("m", acc_multisig.get_m());
    root.put("n", acc_multisig.get_n());
    root.put("self-publickey", acc_multisig.get_pubkey());
    root.put("description", acc_multisig.get_description());

    for(auto& each : acc_multisig.get_cosigner_pubkeys()) {
        pt::ptree pubkey;
        pubkey.put("", each);
        pubkeys.push_back(std::make_pair("", pubkey));
    }
    root.add_child("public-keys", pubkeys);
    root.put("multisig-script", multisig_script);
    root.put("address", acc_multisig.get_address());
    
    pt::write_json(output, root);
    
    return console_result::okay;
}

/************************ listmultisig *************************/

console_result listmultisig::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    // parameter account name check
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    pt::ptree root, nodes;

    if(option_.index) { // according index
        if(option_.index > acc->get_multisig_vec().size())
            throw multisig_index_exception{"multisig index outofbound."};
        
        account_multisig acc_multisig;
        acc->get_multisig(acc_multisig, option_.index);

        pt::ptree node, pubkeys;
        node.put("index", acc_multisig.get_index());
        //node.put("hd_index", acc_multisig.get_hd_index());
        node.put("m", acc_multisig.get_m());
        node.put("n", acc_multisig.get_n());
        node.put("self-publickey", acc_multisig.get_pubkey());
        node.put("description", acc_multisig.get_description());
        for(auto& each : acc_multisig.get_cosigner_pubkeys()) {
            pt::ptree pubkey;
            pubkey.put("", each);
            pubkeys.push_back(std::make_pair("", pubkey));
        }
        node.add_child("public-keys", pubkeys);
        node.put("multisig-script", acc_multisig.get_multisig_script());
        node.put("address", acc_multisig.get_address());
        nodes.push_back(std::make_pair("", node));

    } else {
    
        auto multisig_vec = acc->get_multisig_vec();
            
        for(auto& acc_multisig : multisig_vec) {
            pt::ptree node, pubkeys;
            node.put("index", acc_multisig.get_index());
            //node.put("hd_index", acc_multisig.get_hd_index());
            node.put("m", acc_multisig.get_m());
            node.put("n", acc_multisig.get_n());
            node.put("self-publickey", acc_multisig.get_pubkey());
            node.put("description", acc_multisig.get_description());
            for(auto& each : acc_multisig.get_cosigner_pubkeys()) {
                pt::ptree pubkey;
                pubkey.put("", each);
                pubkeys.push_back(std::make_pair("", pubkey));
            }
            node.add_child("public-keys", pubkeys);
            node.put("multisig-script", acc_multisig.get_multisig_script());
            node.put("address", acc_multisig.get_address());

            nodes.push_back(std::make_pair("", node));
        }   
    }
    root.add_child("multisig", nodes);    
    pt::write_json(output, root);
    
    return console_result::okay;
}

/************************ deletemultisig *************************/
// todo -- delete related address
console_result deletemultisig::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    // parameter account name check
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    //auto acc_multisig = acc->get_multisig();
    account_multisig acc_multisig;
    if(option_.index) { // according index
        if(option_.index > acc->get_multisig_vec().size())
            throw multisig_index_exception{"multisig index outofbound."};
        acc->remove_multisig(acc_multisig, option_.index);
    } else {
        if(option_.public_keys.empty())
            throw multisig_cosigne_exception{"multisig cosigner public key needed."};
        // parameter check
        if( option_.m < 1 )
            throw signature_amount_exception{"signature number less than 1."};
        if( !option_.n || option_.n > 20 )
            throw pubkey_amount_exception{"public key number bigger than 20."};
        if( option_.m > option_.n )
            throw signature_amount_exception{"signature number bigger than public key number."};
        if(option_.self_publickey.empty())
            throw multisig_cosigne_exception{"self public key needed."};
        
        // add self public key into key vector
        auto pubkey = option_.self_publickey;
        if(std::find(option_.public_keys.begin(), option_.public_keys.end(), pubkey) == option_.public_keys.end()) // not found
            option_.public_keys.push_back(pubkey);
        if( option_.n != option_.public_keys.size() )
            throw pubkey_amount_exception{"public key number not match with n."};
        
        acc_multisig.set_m(option_.m);
        acc_multisig.set_n(option_.n);
        acc_multisig.set_pubkey(pubkey);
        acc_multisig.set_cosigner_pubkeys(std::move(option_.public_keys));
        
        if(!(acc->get_multisig(acc_multisig)))
            throw multisig_notfound_exception{"multisig not exists."};
        
        acc->remove_multisig(acc_multisig);
    }

    // change account type
    acc->set_type(account_type::common);
    if(acc->get_multisig_vec().size())
        acc->set_type(account_type::multisignature);
    // flush to db
    blockchain.store_account(acc);

    pt::ptree root, pubkeys;

    root.put("index", acc_multisig.get_index());
    root.put("m", acc_multisig.get_m());
    root.put("n", acc_multisig.get_n());
    root.put("self-publickey", acc_multisig.get_pubkey());
    root.put("description", acc_multisig.get_description());

    for(auto& each : acc_multisig.get_cosigner_pubkeys()) {
        pt::ptree pubkey;
        pubkey.put("", each);
        pubkeys.push_back(std::make_pair("", pubkey));
    }
    root.add_child("public-keys", pubkeys);
    root.put("multisig-script", acc_multisig.get_multisig_script());
    root.put("address", acc_multisig.get_address());
    
    // delete account address
    auto vaddr = blockchain.get_account_addresses(auth_.name);
    if(!vaddr) throw address_list_empty_exception{"empty address list for this account"};

    blockchain.delete_account_address(auth_.name);
    for (auto it = vaddr->begin(); it != vaddr->end();) {
        if (it->get_address() == acc_multisig.get_address()) {
            it = vaddr->erase(it);
            break;
        }
        ++it;
    }

    // restore address
    for (auto& each : *vaddr) {
        auto addr = std::make_shared<bc::chain::account_address>(each);
        blockchain.store_account_address(addr);
    }
    
    pt::write_json(output, root);
    
    return console_result::okay;
}


} // libbitcoin
} // explorer
} // commands
