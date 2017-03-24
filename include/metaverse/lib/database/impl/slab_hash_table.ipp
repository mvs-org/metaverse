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
#ifndef MVS_DATABASE_SLAB_HASH_TABLE_IPP
#define MVS_DATABASE_SLAB_HASH_TABLE_IPP

#include <metaverse/lib/bitcoin.hpp>
#include <metaverse/lib/database/memory/memory.hpp>
#include "../impl/remainder.ipp"
#include "../impl/slab_row.ipp"

namespace libbitcoin {
namespace database {

template <typename KeyType>
slab_hash_table<KeyType>::slab_hash_table(slab_hash_table_header& header,
    slab_manager& manager)
  : header_(header), manager_(manager)
{
}

// This is not limited to storing unique key values. If duplicate keyed values
// are store then retrieval and unlinking will fail as these multiples cannot
// be differentiated. Therefore the database is not currently able to support
// multiple transactions with the same hash, as required by BIP30.
template <typename KeyType>
file_offset slab_hash_table<KeyType>::store(const KeyType& key,
    write_function write, const size_t value_size)
{
	mutex_.lock();
    // Store current bucket value.
    const auto old_begin = read_bucket_value(key);
    slab_row<KeyType> item(manager_, 0);
    const auto new_begin = item.create(key, value_size, old_begin);
    write(item.data());

    // Link record to header.
    link(key, new_begin);
    
    mutex_.unlock();

    // Return position,
    return new_begin + item.value_begin;
}

// This is not limited to store unique key values. If duplicate keyed values
// are store then retrieval and unlinking will fail as these multiples cannot
// be differentiated. Therefore the database is not currently able to support
// multiple transactions with the same hash, as required by BIP30.
template <typename KeyType>
file_offset slab_hash_table<KeyType>::restore(const KeyType& key,
    write_function write, const size_t value_size)
{
	mutex_.lock();
    // Store current bucket value.
    const auto old_begin = read_bucket_value(key);
    slab_row<KeyType> item(manager_, old_begin);
    //const auto new_begin = item.create(key, value_size, old_begin);
    write(item.data());

    // Link record to header.
    //link(key, new_begin);
    
    mutex_.unlock();

    // Return position,
    return old_begin + item.value_begin;
}

// This is limited to returning the first of multiple matching key values.
template <typename KeyType>
const memory_ptr slab_hash_table<KeyType>::find(const KeyType& key) const
{
    // Find start item...
    auto current = read_bucket_value(key);

    // Iterate through list...
    while (current != header_.empty)
    {
        const slab_row<KeyType> item(manager_, current);

        // Found.
        if (item.compare(key))
            return item.data();

        const auto previous = current;
        current = item.next_position();

        // This may otherwise produce an infinite loop here.
        // It indicates that a write operation has interceded.
        // So we must return gracefully vs. looping forever.
        if (previous == current)
            return nullptr;
    }

    return nullptr;
}


// This is limited to returning all the item in the special index.
template <typename KeyType>
std::shared_ptr<std::vector<memory_ptr>> slab_hash_table<KeyType>::find(uint64_t index) const
{
	auto vec_memo = std::make_shared<std::vector<memory_ptr>>();
	// find first item
    auto current = header_.read(index);
    static_assert(sizeof(current) == sizeof(file_offset), "Invalid size");

    // Iterate through list...
    while (current != header_.empty)
    {
        const slab_row<KeyType> item(manager_, current);

        // Found.
        vec_memo->push_back(item.data());

        const auto previous = current;
        current = item.next_position();

        // This may otherwise produce an infinite loop here.
        // It indicates that a write operation has interceded.
        // So we must return gracefully vs. looping forever.
        if (previous == current)
            break;
    }

    return vec_memo;
}


// This is limited to unlinking the first of multiple matching key values.
template <typename KeyType>
bool slab_hash_table<KeyType>::unlink(const KeyType& key)
{
    // Find start item...
    const auto begin = read_bucket_value(key);
    const slab_row<KeyType> begin_item(manager_, begin);

    // If start item has the key then unlink from buckets.
    if (begin_item.compare(key))
    {
        link(key, begin_item.next_position());
        return true;
    }

    // Continue on...
    auto previous = begin;
    auto current = begin_item.next_position();

    // Iterate through list...
    while (current != header_.empty)
    {
        const slab_row<KeyType> item(manager_, current);

        // Found, unlink current item from previous.
        if (item.compare(key))
        {
            release(item, previous);
            return true;
        }

        previous = current;
        current = item.next_position();

        // This may otherwise produce an infinite loop here.
        // It indicates that a write operation has interceded.
        // So we must return gracefully vs. looping forever.
        if (previous == current)
            return false;
    }

    return false;
}

template <typename KeyType>
array_index slab_hash_table<KeyType>::bucket_index(const KeyType& key) const
{
    const auto bucket = remainder(key, header_.size());
    BITCOIN_ASSERT(bucket < header_.size());
    return bucket;
}

template <typename KeyType>
file_offset slab_hash_table<KeyType>::read_bucket_value(
    const KeyType& key) const
{
    const auto value = header_.read(bucket_index(key));
    static_assert(sizeof(value) == sizeof(file_offset), "Invalid size");
    return value;
}

template <typename KeyType>
void slab_hash_table<KeyType>::link(const KeyType& key,
    const file_offset begin)
{
    header_.write(bucket_index(key), begin);
}

template <typename KeyType>
template <typename ListItem>
void slab_hash_table<KeyType>::release(const ListItem& item,
    const file_offset previous)
{
    ListItem previous_item(manager_, previous);
    previous_item.write_next_position(item.next_position());
}

} // namespace database
} // namespace libbitcoin

#endif
