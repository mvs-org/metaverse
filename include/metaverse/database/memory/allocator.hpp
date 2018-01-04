/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
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
#ifndef MVS_DATABASE_ALLOCATOR_HPP
#define MVS_DATABASE_ALLOCATOR_HPP

#include <cstddef>
#include <cstdint>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/define.hpp>
#include <metaverse/database/memory/memory.hpp>

namespace libbitcoin {
namespace database {

#ifdef REMAP_SAFETY

/// This class provides downgradable remap safe access to file-mapped memory.
/// The memory size is unprotected and unmanaged.
class BCD_API allocator
  : public memory
{
public:
    allocator(shared_mutex& mutex);
    ~allocator();

    /// This class is not copyable.
    allocator(const allocator& other) = delete;

    /// Get the address indicated by the pointer.
    uint8_t* buffer();

    /// Increment the pointer the specified number of bytes.
    void increment(size_t value);

protected:
    friend class memory_map;

    /// Set the data member and downgrade the lock.
    void downgrade(uint8_t* data);

private:
    shared_mutex& mutex_;
    uint8_t* data_;
};

#endif // REMAP_SAFETY

} // namespace database
} // namespace libbitcoin

#endif
