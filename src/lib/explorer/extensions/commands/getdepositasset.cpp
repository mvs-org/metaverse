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
#include <metaverse/explorer/extensions/commands/getdepositasset.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ getdepositasset *************************/

console_result getdepositasset::invoke(Json::Value& jv_output,
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

    auto sh_vec = std::make_shared<asset_deposited_balance::list>();
    sync_fetch_asset_deposited(argument_.symbol, blockchain, sh_vec);
    std::sort(sh_vec->begin(), sh_vec->end());

    uint64_t start, end, total_page, tx_count;
    if (argument_.index && argument_.limit) {
        start = (argument_.index - 1) * argument_.limit;
        end = (argument_.index) * argument_.limit;
        if (start >= sh_vec->size() || !sh_vec->size())
            throw argument_legality_exception{"no record in this page"};

        total_page = sh_vec->size() % argument_.limit ? (sh_vec->size() / argument_.limit + 1) : (sh_vec->size() / argument_.limit);
        tx_count = end >= sh_vec->size() ? (sh_vec->size() - start) : argument_.limit ;

    } else if (!argument_.index && !argument_.limit) { // all tx records
        start = 0;
        tx_count = sh_vec->size();
        argument_.index = 1;
        total_page = 1;
    } else {
        throw argument_legality_exception{"invalid limit or index parameter"};
    }


     std::vector<asset_deposited_balance> result(sh_vec->begin() + start, sh_vec->begin() + start + tx_count);

    for (auto& elem: result) {
        auto issued_asset = blockchain.get_issued_asset(elem.symbol);
        if (!issued_asset) {
            continue;
        }
        Json::Value asset_data = json_helper.prop_list(elem, *issued_asset);
        asset_data["status"] = "unspent";
        json_value.append(asset_data);
    }
    
    jv_output["total_page"] = total_page;
    jv_output["current_page"] = argument_.index;
    jv_output["deposit_count"] = tx_count;
    jv_output["deposits"] = json_value;

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

