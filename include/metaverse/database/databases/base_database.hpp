/**
 * Copyright (c) 2011-2015 mvs developers (see AUTHORS)
 *
 * This file is part of mvs-node.
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
#ifndef MVS_DATABASE_BASE_DATABASE_HPP
#define MVS_DATABASE_BASE_DATABASE_HPP

#include <memory>
#include <boost/filesystem.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/define.hpp>
#include <metaverse/database/memory/memory_map.hpp>
#include <metaverse/database/result/base_result.hpp>
#include <metaverse/database/primitives/slab_hash_table.hpp>
#include <metaverse/database/primitives/slab_manager.hpp>

namespace libbitcoin {
namespace database {

/// This enables lookups of bases by hash.
/// An alternative and faster method is lookup from a unique index
/// that is assigned upon storage.
/// This is so we can quickly reconstruct blocks given a list of tx indexes
/// belonging to that block. These are stored with the block.
class BCD_API base_database
{
public:
	
    typedef slab_hash_table<hash_digest> slab_map;
    /// Construct the database.
    base_database(const boost::filesystem::path& map_filename,
        std::shared_ptr<shared_mutex> mutex=nullptr);

    /// Close the database (all threads must first be stopped).
    ~base_database();

    /// Initialize a new base database.
    bool create();

    /// Call before using the database.
    bool start();

    /// Call to signal a stop of current operations.
    bool stop();

    /// Call to unload the memory map.
    bool close();

    /// Fetch base from its hash.
    memory_ptr get(const hash_digest& hash) const;

    /// every subclass should have its own store method, so store method is not define in this class

    /// Delete a base from database.
    void remove(const hash_digest& hash);

    /// Synchronise storage with disk so things are consistent.
    /// Should be done at the end of every block write.
    void sync();
	//slab_map& get_lookup_map() ;
private:

    // Hash table used for looking up txs by hash.
    memory_map lookup_file_;
    slab_hash_table_header lookup_header_;
    slab_manager lookup_manager_;
	
protected:
    slab_map lookup_map_;
};

} // namespace database
} // namespace libbitcoin

#endif
