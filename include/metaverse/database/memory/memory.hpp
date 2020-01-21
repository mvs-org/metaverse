/**
 * Copyright (c) 2019-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2019-2020 metaverse core developers (see MVS-AUTHORS)
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
#ifndef MVS_DATABASE_MEMORY_HPP
#define MVS_DATABASE_MEMORY_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <boost/thread.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/define.hpp>

namespace libbitcoin {
namespace database {

#ifdef REMAP_SAFETY

/// This interface defines remap safe unrestricted access to a memory map.
class BCD_API memory
{
public:
    typedef std::shared_ptr<memory> ptr;

    virtual ~memory() {}

    /// Get the address indicated by the pointer.
    virtual uint8_t* buffer() = 0;

    /// Increment the pointer the specified number of bytes within the record.
    virtual void increment(size_t value) = 0;
};

#endif // REMAP_SAFETY

#ifdef REMAP_SAFETY
    typedef memory::ptr memory_ptr;
    #define REMAP_ADDRESS(ptr) (ptr)->buffer()
    #define REMAP_DOWNGRADE(ptr, data) (ptr)->downgrade(data)
    #define REMAP_INCREMENT(ptr, offset) (ptr)->increment(offset)
    #define REMAP_ACCESSOR(ptr, mutex) std::make_shared<accessor>(mutex, ptr)
    #define REMAP_ALLOCATOR(mutex) std::make_shared<allocator>(mutex)
    #define REMAP_READ(mutex) shared_lock lock(mutex)
    #define REMAP_WRITE(mutex) unique_lock lock(mutex)
#else
    typedef uint8_t* memory_ptr;
    #define REMAP_ADDRESS(ptr) ptr
    #define REMAP_DOWNGRADE(ptr, data)
    #define REMAP_INCREMENT(ptr, offset) ptr += (offset)
    #define REMAP_ACCESSOR(ptr, mutex)
    #define REMAP_ALLOCATOR(mutex)
    #define REMAP_READ(mutex)
    #define REMAP_WRITE(mutex)
#endif // REMAP_SAFETY

#ifdef ALLOCATE_SAFETY
    #define ALLOCATE_READ(mutex) shared_lock lock(mutex)
    #define ALLOCATE_WRITE(mutex) unique_lock lock(mutex)
#else
    #define ALLOCATE_READ(mutex)
    #define ALLOCATE_WRITE(mutex)
#endif // ALLOCATE_SAFETY

} // namespace database
} // namespace libbitcoin

#endif
