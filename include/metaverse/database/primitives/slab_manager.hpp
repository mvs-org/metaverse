/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS)
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
#ifndef MVS_DATABASE_SLAB_MANAGER_HPP
#define MVS_DATABASE_SLAB_MANAGER_HPP

#include <cstddef>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/define.hpp>
#include <metaverse/database/memory/memory.hpp>
#include <metaverse/database/memory/memory_map.hpp>

namespace libbitcoin {
namespace database {

BC_CONSTEXPR size_t minimum_slabs_size = sizeof(file_offset);
BC_CONSTFUNC size_t slab_hash_table_header_size(size_t buckets)
{
    return sizeof(file_offset) + minimum_slabs_size * buckets;
}

/// The slab manager represents a growing collection of various sized
/// slabs of data on disk. It will resize the file accordingly and keep
/// track of the current end pointer so new slabs can be allocated.
class BCD_API slab_manager
{
public:
    slab_manager(memory_map& file, file_offset header_size);

    /// Create slab manager.
    bool create();

    /// Prepare manager for use.
    bool start();

    /// Synchronise the payload size to disk.
    void sync() const;

    /// Allocate a slab and return its position, sync() after writing.
    file_offset new_slab(size_t size);

    /// Return memory object for the slab at the specified position.
    const memory_ptr get(file_offset position) const;

protected:

    /// Get the size of all slabs and size prefix (excludes header).
    file_offset payload_size() const;

private:

    // Read the size of the data from the file.
    void read_size();

    // Write the size of the data from the file.
    void write_size() const;

    // This class is thread and remap safe.
    memory_map& file_;
    const file_offset header_size_;

    // Payload size is protected by mutex.
    file_offset payload_size_;
    mutable shared_mutex mutex_;
};

} // namespace database
} // namespace libbitcoin

#endif
