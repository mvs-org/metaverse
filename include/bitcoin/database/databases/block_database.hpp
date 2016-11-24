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
#ifndef LIBBITCOIN_DATABASE_BLOCK_DATABASE_HPP
#define LIBBITCOIN_DATABASE_BLOCK_DATABASE_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <boost/filesystem.hpp>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/database/define.hpp>
#include <bitcoin/database/memory/memory_map.hpp>
#include <bitcoin/database/primitives/record_manager.hpp>
#include <bitcoin/database/primitives/slab_hash_table.hpp>
#include <bitcoin/database/result/block_result.hpp>

namespace libbitcoin {
namespace database {

/// Stores block_headers each with a list of transaction indexes.
/// Lookup possible by hash or height.
class BCD_API block_database
{
public:
    static const file_offset empty;

    /// Construct the database.
    block_database(const boost::filesystem::path& map_filename,
        const boost::filesystem::path& index_filename,
        std::shared_ptr<shared_mutex> mutex=nullptr);

    /// Close the database (all threads must first be stopped).
    ~block_database();

    /// Initialize a new transaction database.
    bool create();

    /// Call before using the database.
    bool start();

    /// Call to signal a stop of current operations.
    bool stop();

    /// Call to unload the memory map.
    bool close();

    /// Fetch block by height using the index table.
    block_result get(size_t height) const;

    /// Fetch block by hash using the hashtable.
    block_result get(const hash_digest& hash) const;

    /// Store a block in the database.
    void store(const chain::block& block);

    /// Store a block in the database.
    void store(const chain::block& block, size_t height);

    /// Unlink all blocks upwards from (and including) from_height.
    void unlink(size_t from_height);

    /// Synchronise storage with disk so things are consistent.
    /// Should be done at the end of every block write.
    void sync();

    /// The index of the highest existing block, independent of gaps.
    bool top(size_t& out_height) const;

    /// Return the first and last gaps in the blockchain, or false if none.
    bool gap_range(size_t& out_first, size_t& out_last) const;

    /// The index of the first missing block starting from given height.
    bool next_gap(size_t& out_height, size_t start_height) const;

private:
    typedef slab_hash_table<hash_digest> slab_map;

    /// Zeroize the specfied index positions.
    void zeroize(array_index first, array_index count);

    /// Write block hash table position into the block index.
    void write_position(file_offset position, array_index height);

    /// Use block index to get block hash table position from height.
    file_offset read_position(array_index height) const;

    /// Hash table used for looking up blocks by hash.
    memory_map lookup_file_;
    slab_hash_table_header lookup_header_;
    slab_manager lookup_manager_;
    slab_map lookup_map_;

    /// Table used for looking up blocks by height.
    /// Resolves to a position within the slab.
    memory_map index_file_;
    record_manager index_manager_;

    // Guard against concurrent update of a range of block indexes.
    upgrade_mutex mutex_;
};

} // namespace database
} // namespace libbitcoin

#endif
