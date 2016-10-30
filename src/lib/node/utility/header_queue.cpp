/**
 * Copyright (c) 2011-2016 libbitcoin developers (see AUTHORS)
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
#include <bitcoin/node/utility/header_queue.hpp>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <bitcoin/blockchain.hpp>

namespace libbitcoin {
namespace node {

using namespace bc::blockchain;
using namespace bc::chain;
using namespace bc::config;
using namespace bc::message;

header_queue::header_queue(const config::checkpoint::list& checkpoints)
  : height_(0),
    head_(list_.begin()),
    checkpoints_(checkpoints)
{
}

bool header_queue::empty() const
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    shared_lock lock(mutex_);

    return is_empty();
    ///////////////////////////////////////////////////////////////////////////
}

size_t header_queue::size() const
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    shared_lock lock(mutex_);

    return get_size();
    ///////////////////////////////////////////////////////////////////////////
}

size_t header_queue::first_height() const
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    shared_lock lock(mutex_);

    return height_;
    ///////////////////////////////////////////////////////////////////////////
}

size_t header_queue::last_height() const
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    shared_lock lock(mutex_);

    return last();
    ///////////////////////////////////////////////////////////////////////////
}

hash_digest header_queue::last_hash() const
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    shared_lock lock(mutex_);

    return is_empty() ? null_hash : list_.back();
    ///////////////////////////////////////////////////////////////////////////
}

void header_queue::initialize(const checkpoint& check)
{
    initialize(check.hash(), check.height());
}

void header_queue::initialize(const hash_digest& hash, size_t height)
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_upgrade();

    const auto size = 
        (checkpoints_.empty() || height > checkpoints_.back().height()) ? 1 :
        (checkpoints_.back().height() - height + 1);

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    mutex_.unlock_upgrade_and_lock();

    list_.clear();
    list_.reserve(size);
    list_.emplace_back(hash);
    head_ = list_.begin();
    height_ = height;

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////
}

void header_queue::invalidate(size_t first_height, size_t count)
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_upgrade();

    if (first_height < height_ || first_height > last())
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        return;
    }

    const auto first = first_height - height_;
    const auto end = std::min(first + count, get_size());

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    mutex_.unlock_upgrade_and_lock();

    for (size_t index = first; index < end; ++index)
        list_[index] = null_hash;

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////
}

// static
bool header_queue::valid(const hash_digest& hash)
{
    return hash != null_hash;
}

bool header_queue::dequeue(size_t count)
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_upgrade();

    if (count == 0)
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        return true;
    }

    if (is_empty())
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        return false;
    }

    const auto size = get_size();

    if (count > size)
    {
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        mutex_.unlock_upgrade_and_lock();

        BITCOIN_ASSERT(size <= max_size_t - height_);
        height_ += size;

        list_.clear();
        head_ = list_.begin();

        mutex_.unlock();
        //---------------------------------------------------------------------
        return false;
    }
    
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    mutex_.unlock_upgrade_and_lock();

    BITCOIN_ASSERT(count <= max_size_t - height_);
    height_ += count;

    list_.erase(list_.begin(), head_ + count);
    head_ = list_.begin();

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    return true;
}

// This allows the list to become emptied, which breaks the chain.
bool header_queue::dequeue(hash_digest& out_hash, size_t& out_height)
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_upgrade();

    if (is_empty())
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        return false;
    }

    out_height = height_;

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    mutex_.unlock_upgrade_and_lock();

    std::swap(out_hash, *head_);
    ++head_;
    ++height_;

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    return true;
}

bool header_queue::enqueue(headers::ptr message)
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_upgrade();

    // Cannot merge into an empty chain (must be initialized and not cleared).
    if (is_empty())
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        return false;
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    mutex_.unlock_upgrade_and_lock();

    const auto result = merge(message->elements);

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    return result;
}

// private
//-----------------------------------------------------------------------------

// TODO: add PoW validation to reduce importance of intermediate checkpoints.
bool header_queue::merge(const header::list& headers)
{
    // If we exceed capacity the header pointer becomes invalid, so prevent.
    const auto size = get_size();
    list_.erase(list_.begin(), head_);
    list_.reserve(size + headers.size());
    head_ = list_.begin();

    for (const auto& header: headers)
    {
        const auto& new_hash = header.hash();
        const auto next_height = last() + 1;
        const auto& last_hash = is_empty() ? null_hash : list_.back();

        if (!linked(header, last_hash) || !check(new_hash, next_height))
        {
            rollback();
            return false;
        }

        list_.emplace_back(new_hash);
    }

    return true;
}

void header_queue::rollback()
{
    if (!checkpoints_.empty())
    {
        for (auto it = checkpoints_.rbegin(); it != checkpoints_.rend(); ++it)
        {
            auto match = std::find(head_, list_.end(), it->hash());

            if (match != list_.end())
            {
                list_.erase(++match, list_.end());
                return;
            }
        }
    }

    // This assumes that if there are no checkpoints that currently match we
    // trust only the first logical element in the list. This may not be
    // the case depending on how the list has been initialized and/or used.
    if (!is_empty())
    {
        list_.erase(head_ + 1, list_.end());
        list_.erase(list_.begin(), head_);
    }

    head_ = list_.begin();
}

bool header_queue::check(const hash_digest& hash, size_t height) const
{
    return checkpoint::validate(hash, height, checkpoints_);
}

bool header_queue::linked(const chain::header& header,
    const hash_digest& hash) const
{
    return header.previous_block_hash == hash;
}

bool header_queue::is_empty() const
{
    return get_size() == 0;
}

size_t header_queue::get_size() const
{
    hash_list::const_iterator it = head_;
    const auto value = std::distance(it, list_.end());
    return value;
}

size_t header_queue::last() const
{
    return is_empty() ? height_ : height_ + get_size() - 1;
}

} // namespace node
} // namespace libbitcoin
