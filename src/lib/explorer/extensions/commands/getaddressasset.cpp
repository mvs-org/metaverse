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
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/getaddressasset.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ getaddressasset *************************/

console_result getaddressasset::invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node)
{
    auto& aroot = jv_output;
    Json::Value assets;
    std::string symbol;

    auto& blockchain = node.chain_impl();
    if(!blockchain.is_valid_address(argument_.address)) 
        throw address_invalid_exception{"invalid address!"};
    
    // 1. get asset in blockchain
    auto kind = business_kind::asset_transfer;
    std::set<std::string> symbol_set;
    std::vector<asset_detail> asset_vec; // just used asset_detail class

    // make all asset kind vector
    std::vector<business_kind> kind_vec;
    kind_vec.push_back(business_kind::asset_transfer);
    kind_vec.push_back(business_kind::asset_issue);
    
    for (auto kind : kind_vec) {
        // get address unspent asset balance
        auto sh_vec = blockchain.get_address_business_history(argument_.address, kind, business_status::unspent);
        const auto sum = [&](const business_history& bh)
        {
            // get asset info
            std::string symbol;
            uint64_t num;
            if(kind == business_kind::asset_transfer) {
                auto transfer_info = boost::get<chain::asset_transfer>(bh.data.get_data());
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
                asset_vec.push_back(asset_detail(symbol, num, 0, "", argument_.address, ""));
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
    
    for (auto& elem: asset_vec) {
        Json::Value asset_data;
        asset_data["symbol"] = elem.get_symbol();
        symbol = elem.get_symbol();
        if (get_api_version() == 1) {
            asset_data["quantity"] += elem.get_maximum_supply();
        } else {
            asset_data["quantity"] = elem.get_maximum_supply();
        }
        auto issued_asset = blockchain.get_issued_asset(symbol);
        if(issued_asset && get_api_version() == 1) {
            asset_data["decimal_number"] += issued_asset->get_decimal_number();
        }
        if(issued_asset && get_api_version() == 2) {
            asset_data["decimal_number"] = issued_asset->get_decimal_number();
        }
        //asset_data["asset_type"] = elem.detail.get_asset_type();
        //asset_data["issuer"] = elem.detail.get_issuer();
        //asset_data["address"] = elem.detail.get_address();
        //asset_data["description"] = elem.detail.get_description();
        asset_data["address"] = elem.get_address();
        asset_data["status"] = "unspent";
        assets.append(asset_data);
    }    
    aroot["assets"] = assets;

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

