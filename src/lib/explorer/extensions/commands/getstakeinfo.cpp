/**
 * Copyright (c) 2016-2021 mvs developers
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

#include <metaverse/explorer/extensions/commands/getstakeinfo.hpp>
#include <metaverse/explorer/json_helper.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/consensus/witness.hpp>
#include <metaverse/consensus/libdevcore/BasicType.h>

namespace libbitcoin {
namespace explorer {
namespace commands {

/************************ getstakeinfo *************************/

struct stake_info
{
    std::string address;
    uint64_t height;
    uint32_t stake_utxo_available;
    uint32_t stake_utxo_waiting;

    Json::Value to_json() const {
        Json::Value result;
        result["address"] = address;
        result["height"] = height;
        result["stake_utxo_available"] = stake_utxo_available;
        result["stake_utxo_waiting"] = stake_utxo_waiting;
        return result;
    }
};

void get_stake_info(blockchain::block_chain_impl& blockchain, stake_info& stakeinfo)
{
    const auto& address = stakeinfo.address;
    const auto& last_height = stakeinfo.height;

    u256 bits = (u256)HeaderAux::get_minimum_difficulty(last_height, chain::block_version_pos);
    auto header = blockchain.get_prev_block_header(last_height + 1, chain::block_version_pos, true);
    if (header) {
        bits = header->bits;
    }

    wallet::payment_address waddr(address);

    chain::history::list rows;
    rows = blockchain.get_address_history(waddr, false);

    chain::transaction tx_temp;
    uint64_t tx_height;

    for (auto & row : rows) {
        if ((row.spend.hash == null_hash)
            && blockchain.get_transaction(tx_temp, tx_height, row.output.hash)) {

            if (row.value < pos_stake_min_value) {
                continue;
            }

            BITCOIN_ASSERT(row.output.index < tx_temp.outputs.size());
            const auto& output = tx_temp.outputs.at(row.output.index);

            if (!output.is_etp() || output.get_script_address() != address) {
                continue;
            }

            if (!blockchain.check_pos_utxo_capability(
                    bits, last_height, tx_temp, row.output.index, row.output_height, false)) {
                continue;
            }

            bool satisfied = blockchain.check_pos_utxo_height_and_value(bits, row.output_height, last_height, row.value);
            if (satisfied) {
                ++stakeinfo.stake_utxo_available;
            }
            else {
                ++stakeinfo.stake_utxo_waiting;
            }
        }
    }
}

console_result getstakeinfo::invoke(Json::Value& jv_output,
                                     libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto&& address = get_address(argument_.address, blockchain);

    uint64_t last_height = 0;
    if (!blockchain.get_last_height(last_height)) {
        throw block_last_height_get_exception{"query last height failure."};
    }

    stake_info stakeinfo{address, last_height, 0, 0};
    get_stake_info(blockchain, stakeinfo);
    jv_output = stakeinfo.to_json();

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

