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
#ifndef MVS_DATABASE_HISTORY_DATABASE_HPP
#define MVS_DATABASE_HISTORY_DATABASE_HPP

#include <memory>
#include <boost/filesystem.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/define.hpp>
#include <metaverse/database/memory/memory_map.hpp>
#include <metaverse/database/primitives/record_multimap.hpp>

namespace libbitcoin {
namespace database {

struct BCD_API history_statinfo
{
    /// Number of buckets used in the hashtable.
    /// load factor = addrs / buckets
    const size_t buckets;

    /// Total number of unique addresses in the database.
    const size_t addrs;

    /// Total number of rows across all addresses.
    const size_t rows;
};

/// This is a multimap where the key is the Bitcoin address hash,
/// which returns several rows giving the history for that address.
class BCD_API history_database
{
public:
    /// Construct the database.
    history_database(const boost::filesystem::path& lookup_filename,
        const boost::filesystem::path& rows_filename,
        std::shared_ptr<shared_mutex> mutex=nullptr);

    /// Close the database (all threads must first be stopped).
    ~history_database();

    /// Initialize a new history database.
    bool create();

    /// Call before using the database.
    bool start();

    /// Call to signal a stop of current operations.
    bool stop();

    /// Call to unload the memory map.
    bool close();

    /// Add an output row to the key. If key doesn't exist it will be created.
    void add_output(const short_hash& key, const chain::output_point& outpoint,
        uint32_t output_height, uint64_t value);

    /// Add an input to the key. If key doesn't exist it will be created.
    void add_input(const short_hash& key, const chain::output_point& inpoint,
        uint32_t input_height, const chain::input_point& previous);

    /// Delete the last row that was added to key.
    void delete_last_row(const short_hash& key);

    /// Get the output and input points associated with the address hash.
    chain::history_compact::list get(const short_hash& key, size_t limit,
        size_t from_height) const;

    chain::history_compact::list get(size_t from_height) const;

    /// Synchonise with disk.
    void sync();

    /// Return statistical info about the database.
    history_statinfo statinfo() const;

private:
    typedef record_hash_table<short_hash> record_map;
    typedef record_multimap<short_hash> record_multiple_map;

    /// Hash table used for start index lookup for linked list by address hash.
    memory_map lookup_file_;
    record_hash_table_header lookup_header_;
    record_manager lookup_manager_;
    record_map lookup_map_;

    /// List of history rows.
    memory_map rows_file_;
    record_manager rows_manager_;
    record_list rows_list_;
    record_multiple_map rows_multimap_;
};

} // namespace database
} // namespace libbitcoin

#endif
