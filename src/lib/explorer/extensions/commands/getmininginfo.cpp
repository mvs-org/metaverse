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
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/node_method_wrapper.hpp>
#include <metaverse/explorer/extensions/commands/getmininginfo.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ getmininginfo *************************/

console_result getmininginfo::invoke(Json::Value& jv_output,
                                     libbitcoin::server::server_node& node)
{
    administrator_required_checker(node, auth_.name, auth_.auth);

    uint64_t height, rate;
    std::string difficulty;
    bool is_mining;
    uint32_t stake_utxos = 0;

    auto& miner = node.miner();
    miner.get_state(height, rate, difficulty, is_mining, stake_utxos);

    if (get_api_version() <= 2) {
        Json::Value info;
        info["is-mining"] = is_mining;
        info["height"] += height;
        info["rate"] += rate;
        info["difficulty"] = difficulty;
        jv_output["mining-info"] = info;
    }
    else {
        jv_output["is_mining"] = is_mining;
        jv_output["height"] += height;
        jv_output["rate"] += rate;
        jv_output["difficulty"] = difficulty;

        auto& waddr = miner.get_miner_payment_address();
        std::string payment_address = waddr ? waddr.encoded() : "";
        std::string asset_symbol = miner.get_mining_asset_symbol();
        std::string block_version = chain::get_block_version(miner.get_accept_block_version());

        jv_output["payment_address"] = payment_address;
        jv_output["mining_mst"] = asset_symbol;
        jv_output["block_type"] = block_version;

        if (stake_utxos != 0) {
            jv_output["stake_utxo_count"] = stake_utxos;
        }
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

