/*
 * Copyright (c) 2011-2013 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
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
#include <bitcoin/blockchain/block.hpp>

#include <cstdint>
#include <bitcoin/bitcoin.hpp>

namespace libbitcoin {
namespace blockchain {

using namespace chain;

uint64_t block_subsidy(size_t height)
{
    uint64_t subsidy = coin_price(initial_block_reward);
    subsidy >>= (height / reward_interval);
    return subsidy;
}

hash_number block_work(uint32_t bits)
{
    hash_number target;

    if (!target.set_compact(bits))
        return 0;

    if (target == 0)
        return 0;

    // We need to compute 2**256 / (bnTarget+1), but we can't represent 2**256
    // as it's too large for a uint256. However, as 2**256 is at least as large
    // as bnTarget+1, it is equal to ((2**256 - bnTarget - 1) / (bnTarget+1)) + 1,
    // or ~bnTarget / (nTarget+1) + 1.
    return (~target / (target + 1)) + 1;
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
