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
#include <metaverse/explorer/extensions/commands/getasset.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ getasset *************************/

console_result getasset::invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.uppercase_symbol(argument_.symbol);
    
    if (argument_.symbol.size() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw asset_symbol_length_exception{"Illegal asset symbol length."};

    // 1. first search asset in blockchain
    auto sh_vec = blockchain.get_issued_assets();

    auto& aroot = jv_output;
    Json::Value assets;
    std::set<std::string> symbols;
    for (auto& elem: *sh_vec) {
        Json::Value asset_data;

        if (argument_.symbol.empty()) {
            // get rid of duplicate symbols
            if (!symbols.count(elem.get_symbol())) {
                symbols.insert(elem.get_symbol());
                assets.append(elem.get_symbol());
            }
        } else {
            // find out target from blockchain 
            if (elem.get_symbol().compare(argument_.symbol) == 0) {
                asset_data["symbol"] = elem.get_symbol();
                if (get_api_version() == 1) {
                    asset_data["maximum_supply"] += elem.get_maximum_supply();
                    asset_data["decimal_number"] = std::to_string(elem.get_decimal_number());
                    asset_data["secondissue_assetshare_threshold"] += elem.get_secondissue_assetshare_threshold();
                    asset_data["is_secondissue"] = elem.is_asset_secondissue() ? "true" : "false";
                } else {
                    asset_data["maximum_supply"] = elem.get_maximum_supply();
                    asset_data["decimal_number"] = elem.get_decimal_number();
                    asset_data["secondissue_assetshare_threshold"] = elem.get_secondissue_assetshare_threshold();
                    asset_data["is_secondissue"] = elem.is_asset_secondissue();
                }
                asset_data["issuer"] = elem.get_issuer();
                asset_data["address"] = elem.get_address();
                asset_data["description"] = elem.get_description();
                asset_data["status"] = "issued";
                assets.append(asset_data);
            }
        }

    }
    
    if (get_api_version() == 1 && assets.isNull()) { //compatible for v1        
        aroot["assets"] = "";                                                   
    } else {                                                                    
        aroot["assets"] = assets;                                               
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

