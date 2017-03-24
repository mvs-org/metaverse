/**
 * Copyright (c) 2011-2015 mvs developers (see AUTHORS)
 *
 * This file is part of mvs-node.
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
#ifndef MVS_DATABASE_ASSET_DATABASE_HPP
#define MVS_DATABASE_ASSET_DATABASE_HPP
#include <memory>
#include <boost/filesystem.hpp>
#include <metaverse/lib/bitcoin.hpp>
#include <metaverse/lib/database/define.hpp>
#include <metaverse/lib/database/memory/memory_map.hpp>
#include <metaverse/lib/database/result/asset_result.hpp>
#include <metaverse/lib/database/primitives/slab_hash_table.hpp>
#include <metaverse/lib/database/primitives/slab_manager.hpp>
#include <metaverse/lib/database/databases/base_database.hpp>
#include <metaverse/lib/bitcoin/chain/attachment/asset/asset_detail.hpp>

using namespace libbitcoin::chain;

namespace libbitcoin {
namespace database {

/// This enables lookups of assets by hash.
/// An alternative and faster method is lookup from a unique index
/// that is assigned upon storage.
/// This database is used to store asset issued from blockchain(not store local unissued assets)
class BCD_API asset_database: public base_database
{
public:
    /// Construct the database.
    asset_database(const boost::filesystem::path& map_filename,
        std::shared_ptr<shared_mutex> mutex=nullptr);

    /// Close the database (all threads must first be stopped).
    ~asset_database();
	
	/// get asset info by symbol hash
	asset_result get_asset_result(const hash_digest& hash) const;
	/// get all assets in the blockchain
	std::shared_ptr<std::vector<asset_detail>> get_asset_details() const;
    /// Store a asset in the database. Returns a unique index
    /// which can be used to reference the asset.
    void store(const hash_digest& hash, const asset_detail& sp_detail);
};

} // namespace database
} // namespace libbitcoin

#endif
