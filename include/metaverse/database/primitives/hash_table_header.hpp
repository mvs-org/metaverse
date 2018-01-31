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
#ifndef MVS_DATABASE_HASH_TABLE_HEADER_HPP
#define MVS_DATABASE_HASH_TABLE_HEADER_HPP

#include <metaverse/bitcoin.hpp>
#include <metaverse/database/memory/memory_map.hpp>

namespace libbitcoin {
namespace database {

/**
 * Implements contigious memory array with a fixed size elements.
 *
 * File format looks like:
 *
 *  [   size:IndexType   ]
 *  [ [      ...       ] ]
 *  [ [ item:ValueType ] ]
 *  [ [      ...       ] ]
 *
 * Empty elements are represented by the value hash_table_header.empty
 */
template <typename IndexType, typename ValueType>
class hash_table_header
{
public:
    static const ValueType empty;

    hash_table_header(memory_map& file, IndexType buckets);

    // Copy.
    hash_table_header(const hash_table_header&) = delete;
    hash_table_header& operator=(const hash_table_header&) = delete;

    // Move.
    hash_table_header(hash_table_header&&) = delete;
    hash_table_header& operator=(hash_table_header&&) = delete;

    /// Allocate the hash table and populate with empty values.
    bool create();

    /// Must be called before use. Loads the size from the file.
    bool start();

    /// Read item's value.
    ValueType read(IndexType index) const;

    /// Write value to item.
    void write(IndexType index, ValueType value);

    /// The hash table size (bucket count).
    IndexType size() const;

private:

    // Locate the item in the memory map.
    file_offset item_position(IndexType index) const;

    memory_map& file_;
    IndexType buckets_;
    mutable shared_mutex mutex_;
};

} // namespace database
} // namespace libbitcoin

#include <metaverse/database/impl/hash_table_header.ipp>

#endif
