/**
 * Copyright (c) 2011-2020 mvs developers (see AUTHORS)
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
#include <metaverse/database/databases/asset_database.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <boost/filesystem.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/memory/memory.hpp>
//#include <metaverse/database/result/asset_result.hpp>

namespace libbitcoin {
namespace database {

using namespace boost::filesystem;

asset_database::asset_database(const path& map_filename,
    std::shared_ptr<shared_mutex> mutex)
  : base_database(map_filename, mutex)
{
}

// Close does not call stop because there is no way to detect thread join.
asset_database::~asset_database()
{
    close();
}

asset_result asset_database::get_asset_result(const hash_digest& hash) const
{
    const auto memory = get(hash);
    return asset_result(memory);
}
///
std::shared_ptr<std::vector<asset_detail>> asset_database::get_asset_details() const
{
    auto vec_acc = std::make_shared<std::vector<asset_detail>>();
    uint64_t i = 0;
    for ( i = 0; i < get_bucket_count(); i++ ) {
        auto memo = lookup_map_.find(i);
        if(memo->size())
        {
            const auto action = [&](memory_ptr elem)
            {
                const auto memory = REMAP_ADDRESS(elem);
                auto deserial = make_deserializer_unsafe(memory);
                vec_acc->push_back(asset_detail::factory_from_data(deserial));
            };
            std::for_each(memo->begin(), memo->end(), action);
        }
    }
    return vec_acc;
}

void asset_database::store(const hash_digest& hash, const asset_detail& sp_detail)
{
    // Write block data.
    const auto key = hash;
    const auto sp_size = sp_detail.serialized_size();
#ifdef MVS_DEBUG
    log::debug("asset_database::store") << sp_detail.to_string();
#endif
    BITCOIN_ASSERT(sp_size <= max_size_t);
    const auto value_size = static_cast<size_t>(sp_size);

    auto write = [&sp_detail](memory_ptr data)
    {
        auto serial = make_serializer(REMAP_ADDRESS(data));
        serial.write_data(sp_detail.to_data());
    };
    //get_lookup_map().store(key, write, value_size);
    lookup_map_.store(key, write, value_size);
}
} // namespace database
} // namespace libbitcoin
