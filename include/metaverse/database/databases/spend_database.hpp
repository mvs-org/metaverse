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
#ifndef MVS_DATABASE_SPEND_DATABASE_HPP
#define MVS_DATABASE_SPEND_DATABASE_HPP

#include <cstddef>
#include <memory>
#include <boost/filesystem.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/define.hpp>
#include <metaverse/database/primitives/record_hash_table.hpp>
#include <metaverse/database/memory/memory_map.hpp>

namespace libbitcoin {
namespace database {

struct BCD_API spend_statinfo
{
    /// Number of buckets used in the hashtable.
    /// load factor = rows / buckets
    const size_t buckets;

    /// Total number of spend rows.
    const size_t rows;
};

/// This enables you to lookup the spend of an output point, returning
/// the input point. It is a simple map.
class BCD_API spend_database
{
public:
    /// Construct the database.
    spend_database(const boost::filesystem::path& filename,
        std::shared_ptr<shared_mutex> mutex=nullptr);

    /// Close the database (all threads must first be stopped).
    ~spend_database();

    /// Initialize a new spend database.
    bool create();

    /// Call before using the database.
    bool start();

    /// Call to signal a stop of current operations.
    bool stop();

    /// Call to unload the memory map.
    bool close();

    /// Get input spend of an output point.
    chain::spend get(const chain::output_point& outpoint) const;

    /// Store a spend in the database.
    void store(const chain::output_point& outpoint,
        const chain::input_point& spend);

    /// Delete outpoint spend item from database.
    void remove(const chain::output_point& outpoint);

    /// Synchronise storage with disk so things are consistent.
    /// Should be done at the end of every block write.
    void sync();

    /// Return statistical info about the database.
    spend_statinfo statinfo() const;

private:
    typedef record_hash_table<chain::point> record_map;

    // Hash table used for looking up inpoint spends by outpoint.
    memory_map lookup_file_;
    record_hash_table_header lookup_header_;
    record_manager lookup_manager_;
    record_map lookup_map_;
};

} // namespace database
} // namespace libbitcoin

#endif
