/**
 * Copyright (c) 2011-2017 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/node/utility/header_list.hpp>

#include <algorithm>
#include <cstddef>
#include <utility>
#include <metaverse/node/define.hpp>
#include <metaverse/bitcoin/math/limits.hpp>
#include <metaverse/node/utility/check_list.hpp>

namespace libbitcoin {
namespace node {

using namespace bc::chain;
using namespace bc::config;

// Locking is optimized for a single intended caller.
header_list::header_list(size_t slot, const checkpoint& start,
    const checkpoint& stop)
  : height_(safe_add(start.height(), size_t(1))),
    start_(start),
    stop_(stop),
    slot_(slot)
{
    list_.reserve(safe_subtract(stop.height(), start.height()));
}

bool header_list::complete() const
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section.
    shared_lock lock(mutex_);

    return remaining() == 0;
    ///////////////////////////////////////////////////////////////////////////
}

size_t header_list::slot() const
{
    return slot_;
}

size_t header_list::first_height() const
{
    return height_;
}

hash_digest header_list::previous_hash() const
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section.
    shared_lock lock(mutex_);

    return list_.empty() ? start_.hash() : list_.back().hash();
    ///////////////////////////////////////////////////////////////////////////
}

size_t header_list::previous_height() const
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section.
    shared_lock lock(mutex_);

    // This addition is safe.
    return start_.height() + list_.size();
    ///////////////////////////////////////////////////////////////////////////
}

const hash_digest& header_list::stop_hash() const
{
    return stop_.rf_hash();
}

// This is not thread safe, call only after complete.
const chain::header::list& header_list::headers() const
{
    return list_;
}

bool header_list::merge(headers_const_ptr message)
{
    // FIXME.CHENHAO.change as interface
    const auto& headers = message->elements;

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section.
    unique_lock lock(mutex_);

    const auto count = std::min(remaining(), headers.size());
    const auto end = headers.begin() + count;

    for (auto it = headers.begin(); it != end; ++it)
    {
        const auto& header = *it;

        if (!link(header) || !check(header) || !accept(header))
        {
            list_.clear();
            return false;
        }

        list_.push_back(header);
    }

    return true;
    ///////////////////////////////////////////////////////////////////////////
}

////checkpoint::list header_list::to_checkpoints() const
////{
////    ///////////////////////////////////////////////////////////////////////////
////    // Critical Section.
////    shared_lock lock(mutex_);
////
////    if (!complete() || list_.empty())
////        return{};
////
////    checkpoint::list out;
////    out.reserve(list_.size());
////    auto height = start_.height();
////
////    // The height is safe from overflow.
////    for (const auto& header: list_)
////        out.emplace_back(header.hash(), height++);
////
////    return out;
////    ///////////////////////////////////////////////////////////////////////////
////}

// private
//-----------------------------------------------------------------------------

size_t header_list::remaining() const
{
    // This difference is safe from underflow.
    return list_.capacity() - list_.size();
}

bool header_list::link(const chain::header& header) const
{
    // FIXME.CHENHAO.change as interface
    if (list_.empty())
        return header.previous_block_hash == start_.hash();

    // Avoid copying and temporary reference assignment.
    return header.previous_block_hash == list_.back().hash();
}

bool header_list::check(const header& header) const
{
    // This validates is_valid_proof_of_work and is_valid_time_stamp.
    return !header.check();
}

bool header_list::accept(const header& header) const
{
    //// Parallel header download precludes validation of minimum_version,
    //// work_required and median_time_past, however checkpoints are verified.
    ////return !header.accept(...);

    // Verify last checkpoint.
    return remaining() > 1 || header.hash() == stop_.hash();
}

} // namespace node
} // namespace libbitcoin
