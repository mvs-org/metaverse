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
#include <metaverse/explorer/extensions/wallet/getaccountasset.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."

/************************ getaccountasset *************************/

console_result getaccountasset::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    
    // std::shared_ptr<std::vector<business_address_asset>>
    auto sh_local_vec = blockchain.get_account_assets();
    if (0 == sh_local_vec->size()) // no asset found
        throw std::logic_error{"no asset found ?"};

#ifdef MVS_DEBUG
    const auto lc_action = [&](business_address_asset& elem)
    {
        log::info("getasset local db") << elem.to_string();
    };
    std::for_each(sh_local_vec->begin(), sh_local_vec->end(), lc_action);
#endif
    
    pt::ptree aroot;
    pt::ptree assets;
    
    // add local database assets
    for (auto& elem: *sh_local_vec) {
        pt::ptree asset_data;
        asset_data.put("symbol", elem.detail.get_symbol());
        asset_data.put("maximum_supply", elem.detail.get_maximum_supply());
        asset_data.put("asset_type", elem.detail.get_asset_type());
        asset_data.put("issuer", elem.detail.get_issuer());
        asset_data.put("address", elem.detail.get_address());
        asset_data.put("description", elem.detail.get_description());
        asset_data.put("status", "unissued");
        assets.push_back(std::make_pair("", asset_data));
    }
    
    aroot.add_child("assets", assets);
    pt::write_json(output, aroot);
    
    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

