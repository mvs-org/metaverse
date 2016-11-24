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
#include <bitcoin/database/databases/account_address_database.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <boost/filesystem.hpp>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/database/memory/memory.hpp>

namespace libbitcoin {
namespace database {

using namespace boost::filesystem;

BC_CONSTEXPR size_t number_buckets = 10000;
BC_CONSTEXPR size_t header_size = slab_hash_table_header_size(number_buckets);
BC_CONSTEXPR size_t initial_map_file_size = header_size + minimum_slabs_size;

account_address_database::account_address_database(const path& map_filename,
    std::shared_ptr<shared_mutex> mutex)
  : base_database(map_filename, mutex)
{
}

// Close does not call stop because there is no way to detect thread join.
account_address_database::~account_address_database()
{
    close();
}
void account_address_database::store(const hash_digest& hash, const account_address account_address)
{
	// Write block data.
	const auto key = hash;
	const auto acc_size = account_address.serialized_size();

	BITCOIN_ASSERT(acc_size <= max_size_t);
	const auto value_size = static_cast<size_t>(acc_size);

	auto write = [&account_address](memory_ptr data)
	{
		auto serial = make_serializer(REMAP_ADDRESS(data));
		serial.write_data(account_address.to_data());
	};
	get_lookup_map().store(key, write, value_size);
}
account_address_result account_address_database::get_account_address_result(const hash_digest& hash) const
{
	const auto memory = get(hash);
    return account_address_result(memory);
}

} // namespace database
} // namespace libbitcoin
