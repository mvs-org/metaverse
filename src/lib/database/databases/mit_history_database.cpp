/**
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of mvsd.
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
#include <metaverse/database/databases/mit_history_database.hpp>

#include <cstdint>
#include <cstddef>
#include <memory>
#include <boost/filesystem.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/memory/memory.hpp>
#include <metaverse/database/primitives/record_multimap_iterable.hpp>
#include <metaverse/database/primitives/record_multimap_iterator.hpp>

#define  LOG_MIT_HISTORY_DATABASE  "mit_history_database"

namespace libbitcoin {
namespace database {

using namespace boost::filesystem;
using namespace bc::chain;

namespace {
    // Read the height value from the row.
    uint32_t read_height(uint8_t* data)
    {
        return from_little_endian_unsafe<uint32_t>(data);
    };

    // Read the height value from the row.
    uint32_t read_time(uint8_t* data)
    {
        return from_little_endian_unsafe<uint32_t>(data + 4);
    };

    // Read a row from the data for the history list.
    mit read_row(uint8_t* data)
    {
        auto deserial = make_deserializer_unsafe(data);
        deserial.read_4_bytes_little_endian(); // output_height
        deserial.read_4_bytes_little_endian(); // timestamp
        return mit::factory_from_data(deserial);
    };
} // end of namespace anonymous

BC_CONSTEXPR size_t number_buckets = 99999989;
BC_CONSTEXPR size_t header_size = record_hash_table_header_size(number_buckets);
BC_CONSTEXPR size_t initial_lookup_file_size = header_size + minimum_records_size;

BC_CONSTEXPR size_t record_size = hash_table_multimap_record_size<short_hash>();

// output_height + timestamp + mit_status_transfer
BC_CONSTEXPR size_t mit_status_transfer_record_size = 4 + 4 + ASSET_MIT_TRANSFER_FIX_SIZE;
BC_CONSTEXPR size_t row_record_size = hash_table_record_size<hash_digest>(mit_status_transfer_record_size);

mit_history_database::mit_history_database(const path& lookup_filename,
    const path& rows_filename, std::shared_ptr<shared_mutex> mutex)
  : lookup_file_(lookup_filename, mutex),
    lookup_header_(lookup_file_, number_buckets),
    lookup_manager_(lookup_file_, header_size, record_size),
    lookup_map_(lookup_header_, lookup_manager_),
    rows_file_(rows_filename, mutex),
    rows_manager_(rows_file_, 0, row_record_size),
    rows_list_(rows_manager_),
    rows_multimap_(lookup_map_, rows_list_)
{
}

// Close does not call stop because there is no way to detect thread join.
mit_history_database::~mit_history_database()
{
    close();
}

// Create.
// ----------------------------------------------------------------------------

// Initialize files and start.
bool mit_history_database::create()
{
    // Resize and create require a started file.
    if (!lookup_file_.start() ||
        !rows_file_.start())
        return false;

    // These will throw if insufficient disk space.
    lookup_file_.resize(initial_lookup_file_size);
    rows_file_.resize(minimum_records_size);

    if (!lookup_header_.create() ||
        !lookup_manager_.create() ||
        !rows_manager_.create())
        return false;

    // Should not call start after create, already started.
    return
        lookup_header_.start() &&
        lookup_manager_.start() &&
        rows_manager_.start();
}

// Startup and shutdown.
// ----------------------------------------------------------------------------

bool mit_history_database::start()
{
    return
        lookup_file_.start() &&
        rows_file_.start() &&
        lookup_header_.start() &&
        lookup_manager_.start() &&
        rows_manager_.start();
}

bool mit_history_database::stop()
{
    return
        lookup_file_.stop() &&
        rows_file_.stop();
}

bool mit_history_database::close()
{
    return
        lookup_file_.close() &&
        rows_file_.close();
}

void mit_history_database::sync()
{
    lookup_manager_.sync();
    rows_manager_.sync();
}

mit_history_statinfo mit_history_database::statinfo() const
{
    return
    {
        lookup_header_.size(),
        lookup_manager_.count(),
        rows_manager_.count()
    };
}

// ----------------------------------------------------------------------------
void mit_history_database::store(const asset_mit& mit, uint32_t output_height, uint32_t timestamp)
{
    const auto& key_str = mit.get_symbol();
    const data_chunk& data = data_chunk(key_str.begin(), key_str.end());
    const auto key = ripemd160_hash(data);

    auto write = [&mit, &output_height, &timestamp](memory_ptr data)
    {
        auto serial = make_serializer(REMAP_ADDRESS(data));
        serial.write_4_bytes_little_endian(output_height);
        serial.write_4_bytes_little_endian(timestamp);
        serial.write_data(mit.to_short_data());
    };
    rows_multimap_.add_row(key, write);
}

void mit_history_database::delete_last_row(const short_hash& key)
{
    rows_multimap_.delete_last_row(key);
}

std::shared_ptr<asset_mit> mit_history_database::get(const short_hash& key) const
{
    const auto start = rows_multimap_.lookup(key);
    const auto records = record_multimap_iterable(rows_list_, start);

    for (const auto index: records)
    {
        // This obtains a remap safe address pointer against the rows file.
        const auto record = rows_list_.get(index);
        const auto address = REMAP_ADDRESS(record);

        return std::make_shared<mit>(read_row(address));
    }

    return nullptr;
}

std::shared_ptr<std::vector<asset_mit>> mit_history_database::get_history_mits_by_height(
    const short_hash& key, uint32_t start_height, uint32_t end_height,
    uint64_t limit, uint64_t page_number) const
{
    // use map to sort by height, decreasely
    std::map<uint32_t, mit, std::greater<uint32_t>> height_mit_map;

    const auto start = rows_multimap_.lookup(key);
    const auto records = record_multimap_iterable(rows_list_, start);

    uint64_t cnt = 0;
    for (const auto index: records)
    {
        // Stop once we reach the limit (if specified).
        if ((limit > 0) && (height_mit_map.size() >= limit)) {
            break;
        }

        // This obtains a remap safe address pointer against the rows file.
        const auto record = rows_list_.get(index);
        const auto address = REMAP_ADDRESS(record);

        const auto height = read_height(address);

        // Stop once we reach the end (if specified).
        if ((end_height > 0) && (height > end_height)) {
            break;
        }

        // Skip rows below from_height.
        if (height < start_height) {
            continue;
        }

        cnt++;
        if ((limit > 0) && (page_number > 0)
            && ((cnt - 1) / limit) < (page_number - 1)) {
            continue; // skip previous page record
        }

        auto row = read_row(address);
        height_mit_map[height] = row;
    }

    auto result = std::make_shared<std::vector<asset_mit>>();
    for (const auto& pair : height_mit_map) {
        result->emplace_back(std::move(pair.second));
    }
    return result;
}

std::shared_ptr<std::vector<asset_mit>> mit_history_database::get_history_mits_by_time(
    const short_hash& key, uint32_t time_begin, uint32_t time_end,
    uint64_t limit, uint64_t page_number) const
{
    // use map to sort by time, decreasely
    std::map<uint32_t, mit, std::greater<uint32_t>> time_mit_map;

    const auto start = rows_multimap_.lookup(key);
    const auto records = record_multimap_iterable(rows_list_, start);

    uint64_t cnt = 0;
    for (const auto index: records)
    {
        // Stop once we reach the limit (if specified).
        if ((limit > 0) && (time_mit_map.size() >= limit)) {
            break;
        }

        // This obtains a remap safe address pointer against the rows file.
        const auto record = rows_list_.get(index);
        const auto address = REMAP_ADDRESS(record);

        const auto time = read_time(address);

        // Stop once we reach the end (if specified).
        if ((time_end > 0) && (time > time_end)) {
            break;
        }

        // Skip rows below from_height.
        if (time < time_begin) {
            continue;
        }

        cnt++;
        if ((limit > 0) && (page_number > 0)
            && ((cnt - 1) / limit) < (page_number - 1)) {
            continue; // skip previous page record
        }

        auto row = read_row(address);
        time_mit_map[time] = row;
    }

    auto result = std::make_shared<std::vector<asset_mit>>();
    for (const auto& pair : time_mit_map) {
        result->emplace_back(std::move(pair.second));
    }
    return result;
}

} // namespace database
} // namespace libbitcoin

