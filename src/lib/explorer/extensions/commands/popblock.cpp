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

#include <metaverse/explorer/json_helper.hpp>
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/blockchain/block_detail.hpp>
#include <metaverse/explorer/extensions/commands/popblock.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/consensus/witness.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ popblock *************************/
console_result popblock::invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node)
{
    if (argument_.height == 0) {
        throw argument_legality_exception("pop height should be greater than 0.");
    }

    auto& blockchain = node.chain_impl();
    blockchain.set_sync_disabled(true);

    // lock database writing while poping
    unique_lock lock(blockchain.get_mutex());

    uint64_t old_height = 0;
    if (!blockchain.get_last_height(old_height)) {
        throw block_last_height_get_exception("get last height failed.");
    }

    blockchain::block_detail::list released_blocks;
    blockchain.pop_from(released_blocks, argument_.height);

    if (!released_blocks.empty() && consensus::witness::is_dpos_enabled()) {
        uint64_t new_height = 0;
        if (blockchain.get_last_height(new_height) &&
            !consensus::witness::is_in_same_epoch(old_height, new_height)) {
            consensus::witness::get().update_witness_list(new_height);
        }
    }

    blockchain.set_sync_disabled(false);

    jv_output = "pop block from " + std::to_string(argument_.height) + " finished.";

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

