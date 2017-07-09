/**
 * Copyright (c) 2011-2015 mvs developers (see AUTHORS)
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
#include <metaverse/database/databases/account_address_database.hpp>
#include <metaverse/bitcoin/chain/attachment/account/account_address.hpp>

#include <cstdint>
#include <cstddef>
#include <memory>
#include <boost/filesystem.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/memory/memory.hpp>
#include <metaverse/database/primitives/record_multimap_iterable.hpp>
#include <metaverse/database/primitives/record_multimap_iterator.hpp>

namespace libbitcoin {
namespace database {

using namespace boost::filesystem;
using namespace bc::chain;

BC_CONSTEXPR size_t number_buckets = 9997;
BC_CONSTEXPR size_t header_size = record_hash_table_header_size(number_buckets);
BC_CONSTEXPR size_t initial_lookup_file_size = header_size + minimum_records_size;

BC_CONSTEXPR size_t record_size = hash_table_multimap_record_size<short_hash>();

BC_CONSTEXPR size_t address_db_size = ADDRESS_NAME_FIX_SIZE + ADDRESS_PRV_KEY_FIX_SIZE + ADDRESS_PUB_KEY_FIX_SIZE \
	+ ADDRESS_HD_INDEX_FIX_SIZE + ADDRESS_BALANCE_FIX_SIZE + ADDRESS_ALIAS_FIX_SIZE + ADDRESS_ADDRESS_FIX_SIZE
	+ ADDRESS_STATUS_FIX_SIZE; // 222 -- refer account_address.hpp
BC_CONSTEXPR size_t row_record_size = hash_table_record_size<hash_digest>(address_db_size);

account_address_database::account_address_database(const path& lookup_filename,
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
account_address_database::~account_address_database()
{
    close();
}

// Create.
// ----------------------------------------------------------------------------

// Initialize files and start.
bool account_address_database::create()
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

bool account_address_database::start()
{
    return
        lookup_file_.start() &&
        rows_file_.start() &&
        lookup_header_.start() &&
        lookup_manager_.start() &&
        rows_manager_.start();
}

bool account_address_database::stop()
{
    return
        lookup_file_.stop() &&
        rows_file_.stop();
}

bool account_address_database::close()
{
    return
        lookup_file_.close() &&
        rows_file_.close();
}

// ----------------------------------------------------------------------------

void account_address_database::store(const short_hash& key, const account_address& address)
{
	// find the target asset if exist
	auto address_vec = get(key);
	auto pos = std::find_if(address_vec.begin(), address_vec.end(), [&](const account_address& elem){
			return (elem.get_address() == address.get_address());
			});
	
	if (pos == address_vec.end()) { // new item
		// actually store 
		auto write = [&address](memory_ptr data)
		{
			auto serial = make_serializer(REMAP_ADDRESS(data));
			serial.write_data(address.to_data());
		};
		rows_multimap_.add_row(key, write);
	} else { // delete all and recreate all
		*pos = address;
		for(auto& each : address_vec)
			delete_last_row(key);
		for(auto& each : address_vec) {
			auto each_write = [&each](memory_ptr data)
			{
				auto serial = make_serializer(REMAP_ADDRESS(data));
				serial.write_data(each.to_data());
			};
			rows_multimap_.add_row(key, each_write);
		}
	}
}
void account_address_database::delete_last_row(const short_hash& key)
{
    rows_multimap_.delete_last_row(key);
}

account_address::list account_address_database::get(const short_hash& key) const
{
    // Read a row from the data for the account_address list.
    const auto read_row = [](uint8_t* data)
    {
        auto deserial = make_deserializer_unsafe(data);
        return account_address::factory_from_data(deserial);
    };

    account_address::list result;
    const auto start = rows_multimap_.lookup(key);
    const auto records = record_multimap_iterable(rows_list_, start);

    for (const auto index: records)
    {
        // This obtains a remap safe address pointer against the rows file.
        const auto record = rows_list_.get(index);
        const auto address = REMAP_ADDRESS(record);

        result.emplace_back(read_row(address));
    }

    // TODO: we could sort result here.
    return result;
}

std::shared_ptr<account_address> account_address_database::get(const short_hash& key, const std::string& address) const
{
	std::shared_ptr<account_address> addr(nullptr);
    account_address::list result = get(key);
    for (auto element: result)
    {
		if(element.get_address() == address)
		{
			addr = std::make_shared<account_address>(element);
			break;
		}
	}

	return addr;
}
void account_address_database::sync()
{
    lookup_manager_.sync();
    rows_manager_.sync();
}

account_address_statinfo account_address_database::statinfo() const
{
    return
    {
        lookup_header_.size(),
        lookup_manager_.count(),
        rows_manager_.count()
    };
}

} // namespace database
} // namespace libbitcoin

