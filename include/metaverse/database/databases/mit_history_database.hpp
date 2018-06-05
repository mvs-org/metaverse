/**
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
#ifndef MVS_DATABASE_MIT_HISTORY_DATABASE_HPP
#define MVS_DATABASE_MIT_HISTORY_DATABASE_HPP

#include <memory>
#include <boost/filesystem.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/define.hpp>
#include <metaverse/database/memory/memory_map.hpp>
#include <metaverse/database/primitives/record_multimap.hpp>
#include <metaverse/bitcoin/chain/business_data.hpp>

using namespace libbitcoin::chain;

namespace libbitcoin {
namespace database {

struct BCD_API mit_history_statinfo
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
/// which returns several rows giving the mit_history for that address.
class BCD_API mit_history_database
{
public:
    /// Construct the database.
    mit_history_database(const boost::filesystem::path& lookup_filename,
        const boost::filesystem::path& rows_filename,
        std::shared_ptr<shared_mutex> mutex=nullptr);

    /// Close the database (all threads must first be stopped).
    ~mit_history_database();

    /// Initialize a new mit_history database.
    bool create();

    /// Call before using the database.
    bool start();

    /// Call to signal a stop of current operations.
    bool stop();

    /// Call to unload the memory map.
    bool close();

    /// Delete the last row that was added to key.
    void delete_last_row(const short_hash& key);

    /// Synchonise with disk.
    void sync();

    /// Return statistical info about the database.
    mit_history_statinfo statinfo() const;

    void store(const asset_mit_info& mit_info);

    std::shared_ptr<asset_mit_info> get(const short_hash& key) const;

    std::shared_ptr<asset_mit_info::list> get_history_mits_by_height(const short_hash& key,
        uint32_t start_height = 0, uint32_t end_height = 0,
        uint64_t limit = 0, uint64_t page_number = 0) const;

    std::shared_ptr<asset_mit_info::list> get_history_mits_by_time(const short_hash& key,
        uint32_t time_begin, uint32_t time_end,
        uint64_t limit = 0, uint64_t page_number = 0) const;

private:
    typedef record_hash_table<short_hash> record_map;
    typedef record_multimap<short_hash> record_multiple_map;

    /// Hash table used for start index lookup for linked list by address hash.
    memory_map lookup_file_;
    record_hash_table_header lookup_header_;
    record_manager lookup_manager_;
    record_map lookup_map_;

    /// List of mit_history rows.
    memory_map rows_file_;
    record_manager rows_manager_;
    record_list rows_list_;
    record_multiple_map rows_multimap_;
};

} // namespace database
} // namespace libbitcoin

#endif


