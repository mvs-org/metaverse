/**
 * Copyright (c) 2011-2016 libbitcoin developers (see AUTHORS)
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
#include <metaverse/database/memory/allocator.hpp>

#include <cstdint>
#include <cstddef>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/define.hpp>

namespace libbitcoin {
namespace database {

#ifdef REMAP_SAFETY

allocator::allocator(shared_mutex& mutex)
  : mutex_(mutex),
    data_(nullptr)
{
    ///////////////////////////////////////////////////////////////////////////
    // Begin Critical Section

    // Acquire exclusive downgradable lock.
    mutex_.lock_upgrade();
}

uint8_t* allocator::buffer()
{
    return data_;
}

void allocator::increment(size_t value)
{
    BITCOIN_ASSERT_MSG(data_ != nullptr, "Downgrade must be called.");
    BITCOIN_ASSERT((size_t)data_ <= bc::max_size_t - value);
    data_ += value;
}

// protected/friend
void allocator::downgrade(uint8_t* data)
{
    // Downgrade the exclusive lock.
    mutex_.unlock_upgrade_and_lock_shared();

    // Save protected pointer.
    data_ = data;
}

allocator::~allocator()
{
    // Release downgraded (shared) lock.
    mutex_.unlock_shared();

    // End Critical Section
    ///////////////////////////////////////////////////////////////////////////
}

#endif // REMAP_SAFETY

} // namespace database
} // namespace libbitcoin
