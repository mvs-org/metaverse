/*
 * Copyright (c) 2011-2013 libbitcoin developers (see AUTHORS)
 *
 * This file is part of metaverse.
 *
 * metaverse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/blockchain/block.hpp>

#include <cstdint>
#include <metaverse/bitcoin.hpp>

namespace libbitcoin {
namespace blockchain {

using namespace chain;

u256 block_work(u256 bits)
{
	return bits;
}

block::indexes block_locator_indexes(size_t top_height)
{
    BITCOIN_ASSERT(top_height <= bc::max_int64);
    const auto top_height64 = static_cast<int64_t>(top_height);

    block::indexes indexes;

    // Modify the step in the iteration.
    int64_t step = 1;

    // Start at the top of the chain and work backwards.
    for (auto index = top_height64; index > 0; index -= step)
    {
        // Push top 10 indexes first, then back off exponentially.
        if (indexes.size() >= 10)
            step <<= 1;

        indexes.push_back(static_cast<size_t>(index));
    }

    //  Push the genesis block index.
    indexes.push_back(0);
    return indexes;
}

} // namespace blockchain
} // namespace libbitcoin
