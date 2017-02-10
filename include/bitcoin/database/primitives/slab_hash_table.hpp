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
#ifndef MVS_DATABASE_SLAB_HASH_TABLE_HPP
#define MVS_DATABASE_SLAB_HASH_TABLE_HPP

#include <cstddef>
#include <cstdint>
#include <bitcoin/database/memory/memory.hpp>
#include <bitcoin/database/primitives/hash_table_header.hpp>
#include <bitcoin/database/primitives/slab_manager.hpp>

namespace libbitcoin {
namespace database {

typedef hash_table_header<array_index, file_offset> slab_hash_table_header;

/**
 * A hashtable mapping hashes to variable sized values (slabs).
 * Uses a combination of the hash_table and slab_manager.
 *
 * The hash_table is basically a bucket list containing the start
 * value for the slab_row.
 *
 * The slab_manager is used to create linked chains. A header
 * containing the hash of the item, and the next value is stored
 * with each slab.
 *
 *   [ KeyType  ]
 *   [ next:8   ]
 *   [ value... ]
 *
 * If we run manager.sync() before the link() step then we ensure
 * data can be lost but the hashtable is never corrupted.
 * Instead we prefer speed and batch that operation. The user should
 * call allocator.sync() after a series of store() calls.
 */
template <typename KeyType>
class slab_hash_table
{
public:
    typedef std::function<void(memory_ptr)> write_function;

    slab_hash_table(slab_hash_table_header& header, slab_manager& manager);

    /// Store a value. value_size is the requested size for the value.
    /// The provided write() function must write exactly value_size bytes.
    /// Returns the position of the inserted value in the slab_manager.
    file_offset store(const KeyType& key, write_function write,
        const size_t value_size);
	file_offset restore(const KeyType& key,
		write_function write, const size_t value_size);
    /// Find the slab for a given hash. Returns a null pointer if not found.
    const memory_ptr find(const KeyType& key) const;
	std::shared_ptr<std::vector<memory_ptr>> find(uint64_t index) const;
    /// Delete a key-value pair from the hashtable by unlinking the node.
    bool unlink(const KeyType& key);

private:

    // What is the bucket given a hash.
    array_index bucket_index(const KeyType& key) const;

    // What is the slab start position for a chain.
    file_offset read_bucket_value(const KeyType& key) const;

    // Link a new chain into the bucket header.
    void link(const KeyType& key, const file_offset begin);

    // Release node from linked chain.
    template <typename ListItem>
    void release(const ListItem& item, const file_offset previous);

    slab_hash_table_header& header_;
    slab_manager& manager_;
    shared_mutex mutex_;
};

} // namespace database
} // namespace libbitcoin

#include <bitcoin/database/impl/slab_hash_table.ipp>

#endif
