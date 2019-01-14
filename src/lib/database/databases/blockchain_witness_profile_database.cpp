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
#include <metaverse/database/databases/blockchain_witness_profile_database.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <boost/filesystem.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/memory/memory.hpp>

namespace libbitcoin {
namespace database {

using namespace boost::filesystem;

BC_CONSTEXPR size_t number_buckets = 9997;
BC_CONSTEXPR size_t header_size = slab_hash_table_header_size(number_buckets);
BC_CONSTEXPR size_t initial_map_file_size = header_size + minimum_slabs_size;

blockchain_witness_profile_database::blockchain_witness_profile_database(const path& map_filename,
    std::shared_ptr<shared_mutex> mutex)
  : lookup_file_(map_filename, mutex),
    lookup_header_(lookup_file_, number_buckets),
    lookup_manager_(lookup_file_, header_size),
    lookup_map_(lookup_header_, lookup_manager_)
{
}

// Close does not call stop because there is no way to detect thread join.
blockchain_witness_profile_database::~blockchain_witness_profile_database()
{
    close();
}

blockchain_witness_profile_database::key_type
blockchain_witness_profile_database::get_key(uint64_t epoch_height)
{
    blockchain_witness_profile_database::key_type res = {};
    for(auto i = 0; epoch_height > 0; ++i) {
        res[i] = epoch_height & 0xff;
        epoch_height >>= 8;
    }
    return res;
}


// Create.
// ----------------------------------------------------------------------------

// Initialize files and start.
bool blockchain_witness_profile_database::create()
{
    // Resize and create require a started file.
    if (!lookup_file_.start())
        return false;

    // This will throw if insufficient disk space.
    lookup_file_.resize(initial_map_file_size);

    if (!lookup_header_.create() ||
        !lookup_manager_.create())
        return false;

    // Should not call start after create, already started.
    return
        lookup_header_.start() &&
        lookup_manager_.start();
}

// Startup and shutdown.
// ----------------------------------------------------------------------------

// Start files and primitives.
bool blockchain_witness_profile_database::start()
{
    return
        lookup_file_.start() &&
        lookup_header_.start() &&
        lookup_manager_.start();
}

// Stop files.
bool blockchain_witness_profile_database::stop()
{
    return lookup_file_.stop();
}

// Close files.
bool blockchain_witness_profile_database::close()
{
    return lookup_file_.close();
}

// ----------------------------------------------------------------------------

void blockchain_witness_profile_database::remove(uint64_t epoch_height)
{
    const auto key = get_key(epoch_height);
    DEBUG_ONLY(bool success =) lookup_map_.unlink(key);
    BITCOIN_ASSERT(success);
}

void blockchain_witness_profile_database::sync()
{
    lookup_manager_.sync();
}

witness_profile::ptr blockchain_witness_profile_database::get(uint64_t epoch_height) const
{
    const auto key = get_key(epoch_height);
    const auto raw_memory = lookup_map_.find(key);
    if (raw_memory) {
        auto sp_profile = std::make_shared<witness_profile>();
        const auto memory = REMAP_ADDRESS(raw_memory);
        auto deserial = make_deserializer_unsafe(memory);
        sp_profile->from_data(deserial);
        return sp_profile;
    }

    return nullptr;
}

std::shared_ptr<std::map<uint64_t, witness_profile::ptr>>
blockchain_witness_profile_database::get(const std::set<uint64_t>& epoch_heights) const
{
    auto sp_map = std::make_shared<std::map<uint64_t, witness_profile::ptr>>();
    for (auto epoch_height : epoch_heights) {
        (*sp_map)[epoch_height] = get(epoch_height);
    }
    return sp_map;
}

void blockchain_witness_profile_database::store(const witness_profile& profile)
{
    const auto epoch_height = profile.witness_epoch_stat.epoch_start_height;
    const auto key = get_key(epoch_height);

    auto old = get(epoch_height);
    if (old && (*old == profile)) {
        return; // skip duplicate
    }

    const auto serialized_size = profile.serialized_size();
    BITCOIN_ASSERT(serialized_size <= max_size_t);
    const auto value_size = static_cast<size_t>(serialized_size);

    auto write = [&profile](memory_ptr data)
    {
        auto serial = make_serializer(REMAP_ADDRESS(data));
        serial.write_data(profile.to_data());
    };
    lookup_map_.store(key, write, value_size);
}


} // namespace database
} // namespace libbitcoin
