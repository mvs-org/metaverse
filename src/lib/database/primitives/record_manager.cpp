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
#include <metaverse/database/primitives/record_manager.hpp>

#include <cstddef>
#include <stdexcept>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/memory/memory.hpp>
#include <metaverse/database/memory/memory_map.hpp>

/// -- file --
/// [ header ]
/// [ record_count ]
/// [ payload ]

/// -- header (hash table) --
/// [ bucket ]
/// ...
/// [ bucket ]

/// -- payload (fixed size records) --
/// [ record ]
/// ...
/// [ record ]

namespace libbitcoin {
namespace database {

// TODO: guard against overflows.

record_manager::record_manager(memory_map& file, file_offset header_size,
    size_t record_size)
  : file_(file),
    header_size_(header_size),
    record_count_(0),
    record_size_(record_size)
{
}

bool record_manager::create()
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    ALLOCATE_WRITE(mutex_);

    // Existing file record count is nonzero.
    if (record_count_ != 0)
        return false;

    // This currently throws if there is insufficient space.
    file_.resize(header_size_ + record_to_position(record_count_));

    write_count();
    return true;
    ///////////////////////////////////////////////////////////////////////////
}

bool record_manager::start()
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    ALLOCATE_WRITE(mutex_);

    read_count();
    const auto minimum = header_size_ + record_to_position(record_count_);

    // Records size exceeds file size.
    return minimum <= file_.size();
    ///////////////////////////////////////////////////////////////////////////
}

void record_manager::sync()
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    ALLOCATE_WRITE(mutex_);

    write_count();
    ///////////////////////////////////////////////////////////////////////////
}

array_index record_manager::count() const
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    ALLOCATE_READ(mutex_);

    return record_count_;
    ///////////////////////////////////////////////////////////////////////////
}

void record_manager::set_count(const array_index value)
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    ALLOCATE_WRITE(mutex_);

    BITCOIN_ASSERT(value <= record_count_);

    record_count_ = value;
    ///////////////////////////////////////////////////////////////////////////
}

// Return the next index, regardless of the number created.
// The file is thread safe, the critical section is to protect record_count_.
array_index record_manager::new_records(size_t count)
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    ALLOCATE_WRITE(mutex_);

    // Always write after the last index.
    const auto next_record_index = record_count_;

    const size_t position = record_to_position(record_count_ + count);
    const size_t required_size = header_size_ + position;
    file_.reserve(required_size);
    record_count_ += count;

    return next_record_index;
    ///////////////////////////////////////////////////////////////////////////
}

const memory_ptr record_manager::get(array_index record) const
{
    // If record >= count() then we should still be within the file. The
    // condition implies a block has been popped between a guard and this read.
    // The read will be invalidated and should not cause any other fault.

    auto memory = file_.access();
    REMAP_INCREMENT(memory, header_size_ + record_to_position(record));
    return memory;
}

// privates

// Read the count value from the first 32 bits of the file after the header.
void record_manager::read_count()
{
    BITCOIN_ASSERT(header_size_ + sizeof(array_index) <= file_.size());

    // The accessor must remain in scope until the end of the block.
    const auto memory = file_.access();
    const auto count_address = REMAP_ADDRESS(memory) + header_size_;
    record_count_ = from_little_endian_unsafe<array_index>(count_address);
}

// Write the count value to the first 32 bits of the file after the header.
void record_manager::write_count()
{
    BITCOIN_ASSERT(header_size_ + sizeof(array_index) <= file_.size());

    // The accessor must remain in scope until the end of the block.
    auto memory = file_.access();
    auto payload_size_address = REMAP_ADDRESS(memory) + header_size_;
    auto serial = make_serializer(payload_size_address);
    serial.write_little_endian(record_count_);
}

array_index record_manager::position_to_record(file_offset position) const
{
    return (position - sizeof(array_index)) / record_size_;
}

file_offset record_manager::record_to_position(array_index record) const
{
    return sizeof(array_index) + record * record_size_;
}

} // namespace database
} // namespace libbitcoin
