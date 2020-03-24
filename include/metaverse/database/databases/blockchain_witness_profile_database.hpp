/**
 * Copyright (c) 2016-2020 metaverse core developers (see MVS-AUTHORS)
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
#pragma once

#include <memory>
#include <boost/filesystem.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/define.hpp>
#include <metaverse/database/memory/memory_map.hpp>
#include <metaverse/database/result/transaction_result.hpp>
#include <metaverse/database/primitives/slab_hash_table.hpp>
#include <metaverse/database/primitives/slab_manager.hpp>
#include <metaverse/blockchain/profile.hpp>

namespace libbitcoin {
namespace database {

using witness_profile = blockchain::witness_profile;

class BCD_API blockchain_witness_profile_database
{
public:
    /// Construct the database.
    blockchain_witness_profile_database(const boost::filesystem::path& map_filename,
        std::shared_ptr<shared_mutex> mutex=nullptr);

    /// Close the database (all threads must first be stopped).
    ~blockchain_witness_profile_database();

    /// Initialize a new database.
    bool create();

    /// Call before using the database.
    bool start();

    /// Call to signal a stop of current operations.
    bool stop();

    /// Call to unload the memory map.
    bool close();

    witness_profile::ptr get(uint64_t epoch_height) const;

    /// Get witness profiles
    std::shared_ptr<std::map<uint64_t, witness_profile::ptr>>
    get(const std::set<uint64_t>& epoch_heights) const;

    void store(const witness_profile& profile);

    /// Delete from database.
    void remove(uint64_t epoch_height);

    /// Synchronise storage with disk so things are consistent.
    /// Should be done at the end of every block write.
    void sync();

private:
    typedef byte_array<8> key_type;
    typedef slab_hash_table<key_type> slab_map;

    static key_type get_key(uint64_t epoch_height);

    // Hash table used for looking up txs by hash.
    memory_map lookup_file_;
    slab_hash_table_header lookup_header_;
    slab_manager lookup_manager_;
    slab_map lookup_map_;
};

} // namespace database
} // namespace libbitcoin


