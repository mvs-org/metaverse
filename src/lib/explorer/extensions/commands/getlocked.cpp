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

#include <metaverse/explorer/extensions/commands/getlocked.hpp>
#include <metaverse/explorer/json_helper.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ getlocked *************************/

console_result getlocked::invoke(Json::Value& jv_output,
                                     libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto&& address = get_address(argument_.address, blockchain);

    bool is_asset = !is_default_invalid_asset_symbol(option_.asset_symbol);
    if (is_asset) {
        blockchain.uppercase_symbol(option_.asset_symbol);
        check_asset_symbol(option_.asset_symbol);
    }
    auto asset_symbol = is_asset ? option_.asset_symbol : std::string("");

    Json::Value balances;

    auto sh_vec = std::make_shared<locked_balance::list>();
    sync_fetch_locked_balance(address, blockchain, sh_vec, asset_symbol, option_.expiration);
    std::sort(sh_vec->begin(), sh_vec->end());

    for (const auto& balance: *sh_vec) {
        Json::Value json_balance;
        json_balance["address"] = balance.address;
        json_balance["locked_balance"] = balance.locked_value;
        json_balance["locked_height"] = balance.locked_height;
        json_balance["lock_at_height"] = balance.expiration_height - balance.locked_height;
        json_balance["expiration_height"] = balance.expiration_height;
        if (is_asset) {
            json_balance["symbol"] = asset_symbol;
        }

        balances.append(json_balance);
    }

    jv_output = balances;

    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

