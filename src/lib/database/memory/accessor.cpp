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
#include <bitcoin/database/memory/accessor.hpp>

#include <cstdint>
#include <cstddef>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/database/define.hpp>

namespace libbitcoin {
namespace database {

#ifdef REMAP_SAFETY

accessor::accessor(shared_mutex& mutex, uint8_t*& data)
  : mutex_(mutex)
{
    ///////////////////////////////////////////////////////////////////////////
    // Begin Critical Section

    // Acquire shared lock.
    mutex_.lock_shared();

    BITCOIN_ASSERT_MSG(data != nullptr, "Invalid pointer value.");

    // Save protected pointer.
    data_ = data;
}

uint8_t* accessor::buffer()
{
    return data_;
}

void accessor::increment(size_t value)
{
    BITCOIN_ASSERT((size_t)data_ <= bc::max_size_t - value);
    data_ += value;
}

accessor::~accessor()
{
    // Release shared lock.
    mutex_.unlock_shared();

    // End Critical Section
    ///////////////////////////////////////////////////////////////////////////
}

#endif // REMAP_SAFETY

} // namespace database
} // namespace libbitcoin
