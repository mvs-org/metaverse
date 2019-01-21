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
#include <metaverse/explorer/extensions/commands/getstakeinfo.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

/************************ getstakeinfo *************************/

u256 getstakeinfo::get_last_pos_bits(libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    uint64_t height = 0;
    if (!blockchain.get_last_height(height)) {
        throw block_last_height_get_exception{"query last height failure."};
    }

    auto header = blockchain.get_prev_block_header(height + 1, chain::block_version_pos, true);
    if (!header) {
        throw block_header_get_exception{"query last PoS block header failure."};
    }

    return header->bits;
}

console_result getstakeinfo::invoke(Json::Value& jv_output,
                                     libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto&& address = get_address(argument_.address, blockchain);

    wallet::payment_address waddr(address);

    uint64_t last_height = 0;
    blockchain.get_last_height(last_height);

    auto bits = get_last_pos_bits(node);
    auto stake_utxo_count = blockchain.select_utxo_for_staking(bits, last_height, waddr);
    jv_output["address"] = address;
    jv_output["stake_utxo_count"] = stake_utxo_count;

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

