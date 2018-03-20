/**
 * Copyright (c) 2016-2018 mvs developers 
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
#include <metaverse/explorer/extensions/commands/listdids.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ listdids *************************/

console_result listdids::invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node)
{
    auto& aroot = jv_output;
    Json::Value dids;
    
    std::string symbol;
    auto& blockchain = node.chain_impl();
    auto sh_vec = std::make_shared<std::vector<did_detail>>();
    
    if(auth_.name.empty()) { // no account -- list whole dids in blockchain
        sh_vec = blockchain.get_issued_dids();
        
        if (sh_vec->size() == 0) // no did found
            throw did_symbol_notfound_exception{"No did found, please waiting for block synchronizing finish."};

        // add blockchain dids
        for (auto& elem: *sh_vec) {
            Json::Value did_data;
            
            did_data["symbol"] = elem.get_symbol();
            did_data["issuer"] = elem.get_issuer();
            did_data["address"] = elem.get_address();
            did_data["description"] = elem.get_description();
            did_data["status"] = "issued";
            dids.append(did_data);
        }
        
    } else { // list did owned by account
        blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
        auto pvaddr = blockchain.get_account_addresses(auth_.name);
        if(!pvaddr) 
            throw address_list_nullptr_exception{"nullptr for address list"};
        
        for (auto& elem: *sh_vec) {
            Json::Value did_data;
            did_data["symbol"] = elem.get_symbol();
            symbol = elem.get_symbol();
            did_data["issuer"] = elem.get_issuer();
            did_data["description"] = elem.get_description();
            did_data["status"] = "unspent";
            dids.append(did_data);
        }
        // 2. get did in local database
        // shoudl filter all issued did which be stored in local account did database
        sh_vec->clear();
        sh_vec = blockchain.get_issued_dids();
        //std::shared_ptr<std::vector<business_address_did>>
        auto sh_unissued = blockchain.get_account_unissued_dids(auth_.name);          
        for (auto& elem: *sh_unissued) {
            
            auto symbol = elem.detail.get_symbol();            
            auto pos = std::find_if(sh_vec->begin(), sh_vec->end(), [&](const did_detail& elem){
                    return symbol == elem.get_symbol();
                    });
            
            if (pos != sh_vec->end()){ // did already issued in blockchain
                continue;
            } 
            Json::Value did_data;
            did_data["symbol"] = elem.detail.get_symbol();
            did_data["issuer"] = elem.detail.get_issuer();
            did_data["address"] = elem.detail.get_address();
            did_data["description"] = elem.detail.get_description();
            did_data["status"] = "unissued";
            dids.append(did_data);
        }
    }

    if (get_api_version() == 1 && dids.isNull()) { //compatible for v1        
        aroot["dids"] = "";                                                   
    } else {                                                                    
        aroot["dids"] = dids;                                               
    }    

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
