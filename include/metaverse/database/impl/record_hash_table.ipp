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
#ifndef MVS_DATABASE_RECORD_HASH_TABLE_IPP
#define MVS_DATABASE_RECORD_HASH_TABLE_IPP

#include <string>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/memory/memory.hpp>
#include "record_row.ipp"
#include "remainder.ipp"

namespace libbitcoin {
namespace database {

template <typename KeyType>
record_hash_table<KeyType>::record_hash_table(
    record_hash_table_header& header, record_manager& manager)
  : header_(header), manager_(manager)
{
}

// This is not limited to storing unique key values. If duplicate keyed values
// are store then retrieval and unlinking will fail as these multiples cannot
// be differentiated.
template <typename KeyType>
void record_hash_table<KeyType>::store(const KeyType& key,
    const write_function write)
{
    mutex_.lock();
    // Store current bucket value.
    const auto old_begin = read_bucket_value(key);
    record_row<KeyType> item(manager_, 0);
    const auto new_begin = item.create(key, old_begin);
    write(item.data());

    // Link record to header.
    link(key, new_begin);
    mutex_.unlock();
}

// This is limited to returning the first of multiple matching key values.
template <typename KeyType>
const memory_ptr record_hash_table<KeyType>::find(const KeyType& key) const
{
    // Find start item...
    auto current = read_bucket_value(key);

    // Iterate through list...
    while (current != header_.empty)
    {
        const record_row<KeyType> item(manager_, current);

        // Found, return data.
        if (item.compare(key))
            return item.data();

        const auto previous = current;
        current = item.next_index();

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
std::shared_ptr<std::vector<memory_ptr>> record_hash_table<KeyType>::find(array_index index) const
{
    auto vec_memo = std::make_shared<std::vector<memory_ptr>>();
    // find first item
    auto current = header_.read(index);
    static_assert(sizeof(current) == sizeof(array_index), "Invalid size");

    // Iterate through list...
    while (current != header_.empty)
    {
        const record_row<KeyType> item(manager_, current);

        // Found.
        vec_memo->push_back(item.data());

        const auto previous = current;
        current = item.next_index();

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
bool record_hash_table<KeyType>::unlink(const KeyType& key)
{
    // Find start item...
    const auto begin = read_bucket_value(key);
    const record_row<KeyType> begin_item(manager_, begin);

    // If start item has the key then unlink from buckets.
    if (begin_item.compare(key))
    {
        link(key, begin_item.next_index());
        return true;
    }

    // Continue on...
    auto previous = begin;
    auto current = begin_item.next_index();

    // Iterate through list...
    while (current != header_.empty)
    {
        const record_row<KeyType> item(manager_, current);

        // Found, unlink current item from previous.
        if (item.compare(key))
        {
            release(item, previous);
            return true;
        }

        previous = current;
        current = item.next_index();

        // This may otherwise produce an infinite loop here.
        // It indicates that a write operation has interceded.
        // So we must return gracefully vs. looping forever.
        if (previous == current)
            return false;
    }

    return false;
}

template <typename KeyType>
array_index record_hash_table<KeyType>::bucket_index(
    const KeyType& key) const
{
    const auto bucket = remainder(key, header_.size());
    BITCOIN_ASSERT(bucket < header_.size());
    return bucket;
}

template <typename KeyType>
array_index record_hash_table<KeyType>::read_bucket_value(
    const KeyType& key) const
{
    auto value = header_.read(bucket_index(key));
    static_assert(sizeof(value) == sizeof(array_index), "Invalid size");
    return value;
}

template <typename KeyType>
void record_hash_table<KeyType>::link(const KeyType& key,
    const array_index begin)
{
    header_.write(bucket_index(key), begin);
}

template <typename KeyType>
template <typename ListItem>
void record_hash_table<KeyType>::release(const ListItem& item,
    const file_offset previous)
{
    ListItem previous_item(manager_, previous);
    previous_item.write_next_index(item.next_index());
}

} // namespace database
} // namespace libbitcoin

#endif
