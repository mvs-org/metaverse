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
#include <metaverse/explorer/extensions/wallet/listassets.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."

/************************ listassets *************************/

console_result listassets::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    pt::ptree aroot;
    pt::ptree assets;
    if(auth_.name.empty()) { // no account -- list whole assets in blockchain
        //blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
        // std::shared_ptr<std::vector<asset_detail>> 
        auto sh_vec = blockchain.get_issued_assets();
        
        if ( 0 == sh_vec->size()) // no asset found
            throw std::logic_error{"no asset found ?"};

#ifdef MVS_DEBUG
        const auto action = [&](asset_detail& elem)
        {
            log::info("listassets blockchain") << elem.to_string();
        };
        std::for_each(sh_vec->begin(), sh_vec->end(), action);
#endif

        // add blockchain assets
        for (auto& elem: *sh_vec) {
            pt::ptree asset_data;
            asset_data.put("symbol", elem.get_symbol());
            asset_data.put("amount", elem.get_maximum_supply());
            //asset_data.put("address", elem.get_address());
            asset_data.put("status", "issued");
            assets.push_back(std::make_pair("", asset_data));
        }
        
    } else { // list asset owned by account
        blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
        auto pvaddr = blockchain.get_account_addresses(auth_.name);
        if(!pvaddr) 
            throw std::logic_error{"nullptr for address list"};
        
        // 1. get asset in blockchain
        auto kind = business_kind::asset_transfer;
        std::set<std::string> symbol_set;
        std::vector<asset_detail> asset_vec; // just used asset_detail class to store asset infor

        // make all asset kind vector
        std::vector<business_kind> kind_vec;
        kind_vec.push_back(business_kind::asset_transfer);
        kind_vec.push_back(business_kind::asset_issue);
        
        for (auto kind : kind_vec) {
            // get address unspent asset balance
            for (auto& each : *pvaddr){
                auto sh_vec = blockchain.get_address_business_history(each.get_address(), kind, business_status::unspent);
                const auto sum = [&](const business_history& bh)
                {
                    // get asset info
                    std::string symbol;
                    uint64_t num;
                    if(kind == business_kind::asset_transfer) {
                        auto transfer_info = boost::get<asset_transfer>(bh.data.get_data());
                        symbol = transfer_info.get_address();
                        num = transfer_info.get_quantity();
                    } else { // asset issued
                        auto asset_info = boost::get<asset_detail>(bh.data.get_data());
                        symbol = asset_info.get_symbol();
                        num = asset_info.get_maximum_supply();
                    }
                    
                    // update asset quantity
                    auto r = symbol_set.insert(symbol);
                    if(r.second) { // new symbol
                        asset_vec.push_back(asset_detail(symbol, num, 0, "", each.get_address(), ""));
                    } else { // already exist
                        const auto add_num = [&](asset_detail& elem)
                        {
                            if( 0 == symbol.compare(elem.get_symbol()) )
                                elem.set_maximum_supply(elem.get_maximum_supply()+num);
                        };
                        std::for_each(asset_vec.begin(), asset_vec.end(), add_num);
                    }
                };
                std::for_each(sh_vec->begin(), sh_vec->end(), sum);
            }
        } 
        
        for (auto& elem: asset_vec) {
            pt::ptree asset_data;
            asset_data.put("symbol", elem.get_symbol());
            asset_data.put("amount", elem.get_maximum_supply());
            //asset_data.put("address", elem.get_address());
            asset_data.put("status", "unspent");
            assets.push_back(std::make_pair("", asset_data));
        }
        // 2. get asset in local database
        // shoudl filter all issued asset which be stored in local account asset database
        auto sh_vec = blockchain.get_issued_assets();
        for (auto& elem: *sh_vec) {
            symbol_set.insert(elem.get_symbol());
        }
        //std::shared_ptr<std::vector<business_address_asset>>
        auto sh_unissued = blockchain.get_account_unissued_assets(auth_.name);        
        for (auto& elem: *sh_unissued) {
            
            auto symbol = elem.detail.get_symbol();
            auto r = symbol_set.insert(symbol);
            if(!r.second) { // asset already issued in blockchain
                continue; 
            }

            pt::ptree asset_data;
            asset_data.put("symbol", elem.detail.get_symbol());
            asset_data.put("amount", elem.detail.get_maximum_supply());
            //asset_data.put("address", "");
            asset_data.put("status", "unissued");
            assets.push_back(std::make_pair("", asset_data));
        }
    }
    
    aroot.add_child("assets", assets);
    pt::write_json(output, aroot);
    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

