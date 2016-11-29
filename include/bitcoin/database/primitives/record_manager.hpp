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
#ifndef MVS_DATABASE_RECORD_MANAGER_HPP
#define MVS_DATABASE_RECORD_MANAGER_HPP

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/database/define.hpp>
#include <bitcoin/database/memory/memory.hpp>
#include <bitcoin/database/memory/memory_map.hpp>

namespace libbitcoin {
namespace database {

BC_CONSTEXPR size_t minimum_records_size = sizeof(array_index);
BC_CONSTFUNC size_t record_hash_table_header_size(size_t buckets)
{
    return sizeof(array_index) + minimum_records_size * buckets;
}

/// The record manager represents a collection of fixed size chunks of
/// data referenced by an index. The file will be resized accordingly
/// and the total number of records updated so new chunks can be allocated.
/// It also provides logical record mapping to the record memory address.
class BCD_API record_manager
{
public:
    record_manager(memory_map& file, file_offset header_size,
        size_t record_size);

    /// Create record manager.
    bool create();

    /// Prepare manager for usage.
    bool start();

    /// Synchronise to disk.
    void sync();

    /// The number of records in this container.
    array_index count() const;

    /// Change the number of records of this container (truncation).
    void set_count(const array_index value);

    /// Allocate records and return first logical index, sync() after writing.
    array_index new_records(size_t count);

    /// Return memory object for the record at the specified index.
    const memory_ptr get(array_index record) const;

private:

    // The record index of a disk position.
    array_index position_to_record(file_offset position) const;

    // The disk position of a record index.
    file_offset record_to_position(array_index record) const;

    // Read the count of the records from the file.
    void read_count();

    // Write the count of the records from the file.
    void write_count();

    // This class is thread and remap safe.
    memory_map& file_;
    const file_offset header_size_;

    // Payload size is protected by mutex.
    array_index record_count_;
    mutable shared_mutex mutex_;

    // Records are fixed size.
    const size_t record_size_;
};

} // namespace database
} // namespace libbitcoin

#endif
