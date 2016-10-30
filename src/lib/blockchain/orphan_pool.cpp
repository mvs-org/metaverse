/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
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
#include <bitcoin/blockchain/orphan_pool.hpp>

#include <algorithm>
#include <cstddef>
#include <bitcoin/blockchain/block_detail.hpp>

namespace libbitcoin {
namespace blockchain {

orphan_pool::orphan_pool(size_t capacity)
  : buffer_(capacity)
{
}

// There is no validation whatsoever of the block up to this pont.
bool orphan_pool::add(block_detail::ptr block)
{
    const auto& header = block->actual()->header;

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_upgrade();

    // No duplicates allowed.
    if (exists(header))
    {
        mutex_.unlock_upgrade();
        //-----------------------------------------------------------------
        return false;
    }

    const auto old_size = buffer_.size();
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    mutex_.unlock_upgrade_and_lock();
    buffer_.push_back(block);
    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    log::debug(LOG_BLOCKCHAIN)
        << "Orphan pool added block [" << encode_hash(block->hash())
        << "] previous [" << encode_hash(header.previous_block_hash)
        << "] old size (" << old_size << ").";

    return true;
}

void orphan_pool::remove(block_detail::ptr block)
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_upgrade();

    const auto it = std::find(buffer_.begin(), buffer_.end(), block);

    if (it == buffer_.end())
    {
        mutex_.unlock_upgrade();
        //-----------------------------------------------------------------
        return;
    }

    const auto old_size = buffer_.size();
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    mutex_.unlock_upgrade_and_lock();
    buffer_.erase(it);
    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    log::debug(LOG_BLOCKCHAIN)
        << "Orphan pool removed block [" << encode_hash(block->hash())
        << "] old size (" << old_size << ").";
}

// TODO: use hash table pool to eliminate this O(n^2) search.
void orphan_pool::filter(message::get_data::ptr message) const
{
    auto& inventories = message->inventories;

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    shared_lock lock(mutex_);

    for (auto it = inventories.begin(); it != inventories.end();)
        if (it->is_block_type() && exists(it->hash))
            it = inventories.erase(it);
        else
            ++it;
    ///////////////////////////////////////////////////////////////////////////
}

block_detail::list orphan_pool::trace(block_detail::ptr end) const
{
    block_detail::list trace;
    trace.reserve(buffer_.size());
    trace.push_back(end);
    auto& hash = end->actual()->header.previous_block_hash;

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_shared();

    for (auto it = find(hash); it != buffer_.end(); it = find(hash))
    {
        trace.push_back(*it);
        hash = (*it)->actual()->header.previous_block_hash;
    }

    mutex_.unlock_shared();
    ///////////////////////////////////////////////////////////////////////////

    BITCOIN_ASSERT(!trace.empty());
    std::reverse(trace.begin(), trace.end());
    trace.shrink_to_fit();
    return trace;
}

block_detail::list orphan_pool::unprocessed() const
{
    block_detail::list unprocessed;
    unprocessed.reserve(buffer_.size());

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_shared();

    // Earlier blocks enter pool first, so reversal helps avoid fragmentation.
    for (auto it = buffer_.rbegin(); it != buffer_.rend(); ++it)
        if (!(*it)->processed())
            unprocessed.push_back(*it);

    mutex_.unlock_shared();
    ///////////////////////////////////////////////////////////////////////////

    unprocessed.shrink_to_fit();
    return unprocessed;
}

// private
//-----------------------------------------------------------------------------

bool orphan_pool::exists(const hash_digest& hash) const
{
    const auto match = [&hash](const block_detail::ptr& entry)
    {
        return hash == entry->hash();
    };

    return std::any_of(buffer_.begin(), buffer_.end(), match);
}

bool orphan_pool::exists(const chain::header& header) const
{
    const auto match = [&header](const block_detail::ptr& entry)
    {
        return header == entry->actual()->header;
    };

    return std::any_of(buffer_.begin(), buffer_.end(), match);
}

orphan_pool::const_iterator orphan_pool::find(const hash_digest& hash) const
{
    const auto match = [&hash](const block_detail::ptr& entry)
    {
        return hash == entry->hash();
    };

    return std::find_if(buffer_.begin(), buffer_.end(), match);
}

} // namespace blockchain
} // namespace libbitcoin
