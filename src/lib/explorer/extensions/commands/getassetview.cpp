/**
 * Copyright (c) 2019-2020 mvs developers
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
#include <metaverse/explorer/extensions/commands/getassetview.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ getassetview *************************/

console_result getassetview::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();

    if (!argument_.symbol.empty()) {
        // check asset symbol
        blockchain.uppercase_symbol(argument_.symbol);
        check_asset_symbol(argument_.symbol);
    }

       // page limit & page index paramenter check
    if (!argument_.index)
        throw argument_legality_exception{"page index parameter cannot be zero"};
    if (!argument_.limit)
        throw argument_legality_exception{"page record limit parameter cannot be zero"};
    if (argument_.limit > 100)
        throw argument_legality_exception{"page record limit cannot be bigger than 100."};

    Json::Value json_value;
    auto json_helper = config::json_helper(get_api_version());

    uint64_t start, end, total_page, tx_count;
    auto calc_range = [&](uint64_t record_size){
        if (record_size == 0|| (!argument_.index && !argument_.limit)) { // all tx records
            start = 0;
            tx_count = record_size;
            argument_.index = 1;
            total_page = 1;
        } else if (argument_.index && argument_.limit) {
            start = (argument_.index - 1) * argument_.limit;
            end = (argument_.index) * argument_.limit;
            if (start >= record_size || record_size == 0)
                throw argument_legality_exception{"no record in this page"};

            total_page = record_size % argument_.limit ? (record_size / argument_.limit + 1) : (record_size / argument_.limit);
            tx_count = end >= record_size ? (record_size - start) : argument_.limit ;

        } else {
            throw argument_legality_exception{"invalid limit or index parameter"};
        }
    };

    if (option_.is_deposit){
        auto sh_vec = sync_fetch_asset_deposited_view(argument_.symbol, blockchain);
        if (!sh_vec->empty()){
            std::sort(sh_vec->begin(), sh_vec->end());
        }

        calc_range(sh_vec->size());

        std::vector<asset_deposited_balance> result(sh_vec->begin() + start, sh_vec->begin() + start + tx_count);
        for (auto &elem : result){
            auto issued_asset = blockchain.get_issued_asset(elem.symbol);
            if (!issued_asset){
                    continue;
            }
            Json::Value asset_data = json_helper.prop_list(elem, *issued_asset);
            asset_data["status"] = "unspent";
            json_value.append(asset_data);
            
        }
    }
    else
    {
        auto sh_vec = sync_fetch_asset_view(argument_.symbol, blockchain);
        if (!sh_vec->empty()){
            std::sort(sh_vec->begin(), sh_vec->end());
        }
        
        calc_range(sh_vec->size());
        std::vector<asset_balances> result(sh_vec->begin() + start, sh_vec->begin() + start + tx_count);
        for (auto &elem : result){
            Json::Value asset_data = json_helper.prop_list(elem);
            asset_data["status"] = "unspent";
            json_value.append(asset_data);
        }
    }

    jv_output["total_page"] = total_page;
    jv_output["current_page"] = argument_.index;
    jv_output["view_count"] = tx_count;
    jv_output["views"] = json_value;

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

