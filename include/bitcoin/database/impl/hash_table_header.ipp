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
#ifndef MVS_DATABASE_HASH_TABLE_HEADER_IPP
#define MVS_DATABASE_HASH_TABLE_HEADER_IPP

#include <cstring>
#include <stdexcept>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/database/memory/memory.hpp>

namespace libbitcoin {
namespace database {

// If the bucket count is zero we trust what is read from the file.
static BC_CONSTEXPR size_t trust_file_bucket_count = 0;

// This VC++ workaround is OK because ValueType must be unsigned. 
//static constexpr ValueType empty = std::numeric_limits<ValueType>::max();
template <typename IndexType, typename ValueType>
const ValueType hash_table_header<IndexType, ValueType>::empty =
    (ValueType)bc::max_uint64;

template <typename IndexType, typename ValueType>
hash_table_header<IndexType, ValueType>::hash_table_header(memory_map& file)
  : hash_table_header(file, trust_file_bucket_count)
{
}

template <typename IndexType, typename ValueType>
hash_table_header<IndexType, ValueType>::hash_table_header(memory_map& file,
    IndexType buckets)
  : file_(file), buckets_(buckets)
{
    ////static_assert(empty == (ValueType)0xffffffffffffffff,
    ////    "Unexpected value for empty sentinel.");
    BITCOIN_ASSERT_MSG(empty == (ValueType)0xffffffffffffffff,
        "Unexpected value for empty sentinel.");

    static_assert(std::is_unsigned<ValueType>::value,
        "Hash table header requires unsigned type.");
}

template <typename IndexType, typename ValueType>
bool hash_table_header<IndexType, ValueType>::create()
{
    // Cannot create zero-sized hash table.
    // If buckets_ == 0 we trust what is read from the file.
    if (buckets_ == 0)
        return false;

    // Calculate the minimum file size.
    const auto minimum_file_size = item_position(buckets_);

    // The accessor must remain in scope until the end of the block.
    const auto memory = file_.resize(minimum_file_size);
    const auto buckets_address = REMAP_ADDRESS(memory);
    auto serial = make_serializer(buckets_address);
    serial.write_little_endian(buckets_);

    // optimized fill implementation
    // This optimization makes it possible to debug full size headers.
    const auto start = buckets_address + sizeof(IndexType);
    memset(start, 0xff, buckets_ * sizeof(ValueType));

    // rationalized fill implementation
    ////for (IndexType index = 0; index < buckets_; ++index)
    ////    serial.write_little_endian(empty);
    return true;
}

// If false header file indicates incorrect size.
template <typename IndexType, typename ValueType>
bool hash_table_header<IndexType, ValueType>::start()
{
    const auto minimum_file_size = item_position(buckets_);

    // Header file is too small.
    if (minimum_file_size > file_.size())
        return false;

    // The accessor must remain in scope until the end of the block.
    const auto memory = file_.access();
    const auto buckets_address = REMAP_ADDRESS(memory);

    // Does not require atomicity (no concurrency during start).
    const auto buckets = from_little_endian_unsafe<IndexType>(buckets_address);

    // If buckets_ == 0 we trust what is read from the file.
    return buckets_ == 0 || buckets == buckets_;
}

template <typename IndexType, typename ValueType>
ValueType hash_table_header<IndexType, ValueType>::read(IndexType index) const
{
    // This is not runtime safe but test is avoided as an optimization.
    BITCOIN_ASSERT(index < buckets_);
    
    // The accessor must remain in scope until the end of the block.
    const auto memory = file_.access();
    const auto value_address = REMAP_ADDRESS(memory) + item_position(index);

    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    shared_lock lock(mutex_);
    return from_little_endian_unsafe<ValueType>(value_address);
    ///////////////////////////////////////////////////////////////////////////
}

template <typename IndexType, typename ValueType>
void hash_table_header<IndexType, ValueType>::write(IndexType index,
    ValueType value)
{
    // This is not runtime safe but test is avoided as an optimization.
    BITCOIN_ASSERT(index < buckets_);
    
    // The accessor must remain in scope until the end of the block.
    const auto memory = file_.access();
    const auto value_address = REMAP_ADDRESS(memory) + item_position(index);
    auto serial = make_serializer(value_address);

    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    unique_lock lock(mutex_);
    serial.template write_little_endian<ValueType>(value);
    ///////////////////////////////////////////////////////////////////////////
}

template <typename IndexType, typename ValueType>
IndexType hash_table_header<IndexType, ValueType>::size() const
{
    return buckets_;
}

template <typename IndexType, typename ValueType>
file_offset hash_table_header<IndexType, ValueType>::item_position(
    IndexType index) const
{
    return sizeof(IndexType) + index * sizeof(ValueType);
}

} // namespace database
} // namespace libbitcoin

#endif
