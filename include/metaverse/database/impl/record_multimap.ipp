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
#ifndef MVS_DATABASE_RECORD_MULTIMAP_IPP
#define MVS_DATABASE_RECORD_MULTIMAP_IPP

#include <string>
#include <metaverse/database/memory/memory.hpp>

namespace libbitcoin {
namespace database {

template <typename KeyType>
record_multimap<KeyType>::record_multimap(record_hash_table_type& map,
    record_list& records)
  : map_(map), records_(records)
{
}

template <typename KeyType>
array_index record_multimap<KeyType>::lookup(const KeyType& key) const
{
    const auto start_info = map_.find(key);

    if (!start_info)
        return records_.empty;

    const auto address = REMAP_ADDRESS(start_info);

    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    unique_lock lock(mutex_);
    return from_little_endian_unsafe<array_index>(address);
    ///////////////////////////////////////////////////////////////////////////
}

template <typename KeyType>
std::shared_ptr<std::vector<array_index>> record_multimap<KeyType>::lookup(array_index index) const
{
    auto sh_ret_vec = std::make_shared<std::vector<array_index>>();
    auto sh_vec = map_.find(index);

    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    unique_lock lock(mutex_);
    for(auto each : *sh_vec) {
        const auto address = REMAP_ADDRESS(each);
        sh_ret_vec->push_back(from_little_endian_unsafe<array_index>(address));
    }
    ///////////////////////////////////////////////////////////////////////////
    return sh_ret_vec;
}

template <typename KeyType>
void record_multimap<KeyType>::add_row(const KeyType& key,
    write_function write)
{
    const auto start_info = map_.find(key);

    if (!start_info)
    {
        create_new(key, write);
        return;
    }

    // This forwards a memory object.
    add_to_list(start_info, write);
}

template <typename KeyType>
void record_multimap<KeyType>::add_to_list(memory_ptr start_info,
    write_function write)
{
    const auto address = REMAP_ADDRESS(start_info);

    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_shared();
    const auto old_begin = from_little_endian_unsafe<array_index>(address);
    mutex_.unlock_shared();
    ///////////////////////////////////////////////////////////////////////////

    const auto new_begin = records_.insert(old_begin);

    // The records_ and start_info remap safe pointers are in distinct files.
    write(records_.get(new_begin));

    auto serial = make_serializer(address);

    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    unique_lock lock(mutex_);
    serial.template write_little_endian<array_index>(new_begin);
    ///////////////////////////////////////////////////////////////////////////
}

template <typename KeyType>
void record_multimap<KeyType>::delete_last_row(const KeyType& key)
{
    const auto start_info = map_.find(key);
    if (!start_info) {
        return;
    }
    BITCOIN_ASSERT_MSG(start_info, "The row to delete was not found.");

    auto address = REMAP_ADDRESS(start_info);

    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_shared();
    const auto old_begin = from_little_endian_unsafe<array_index>(address);
    mutex_.unlock_shared();
    ///////////////////////////////////////////////////////////////////////////

    BITCOIN_ASSERT(old_begin != records_.empty);
    const auto new_begin = records_.next(old_begin);

    if (new_begin == records_.empty)
    {
        // Free existing remap pointer to prevent deadlock in map_.unlink.
        address = nullptr;

        DEBUG_ONLY(bool success =) map_.unlink(key);
        BITCOIN_ASSERT(success);
        return;
    }

    auto serial = make_serializer(address);

    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    unique_lock lock(mutex_);
    serial.template write_little_endian<array_index>(new_begin);
    ///////////////////////////////////////////////////////////////////////////
}

template <typename KeyType>
void record_multimap<KeyType>::create_new(const KeyType& key,
    write_function write)
{
    const auto first = records_.create();
    write(records_.get(first));

    const auto write_start_info = [this, first](memory_ptr data)
    {
        auto serial = make_serializer(REMAP_ADDRESS(data));

        // Critical Section
        ///////////////////////////////////////////////////////////////////////////
        unique_lock lock(mutex_);
        serial.template write_little_endian<array_index>(first);
        ///////////////////////////////////////////////////////////////////////////
    };
    map_.store(key, write_start_info);
}

} // namespace database
} // namespace libbitcoin

#endif
