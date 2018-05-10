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
#include <metaverse/database/databases/account_database.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <boost/filesystem.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/memory/memory.hpp>
#include <metaverse/database/data_base.hpp>
#include <metaverse/bitcoin/utility/path.hpp>

namespace libbitcoin {
namespace database {

using namespace boost::filesystem;

BC_CONSTEXPR size_t number_buckets = 9997; // copy from base_database.cpp
//BC_CONSTEXPR size_t header_size = slab_hash_table_header_size(number_buckets);
//BC_CONSTEXPR size_t initial_map_file_size = header_size + minimum_slabs_size;

account_database::account_database(const path& map_filename,
    std::shared_ptr<shared_mutex> mutex)
  : base_database(map_filename, mutex)
{
}

// Close does not call stop because there is no way to detect thread join.
account_database::~account_database()
{
    close();
}
void account_database::set_admin(const std::string& name, const std::string& passwd)
{
	// create admin account if not exists
	const auto hash = get_hash(name);
	if( nullptr == get(hash)) {
		account acc;
		acc.set_name(name);
		acc.set_passwd(passwd);
		acc.set_priority(account_priority::administrator);
		store(hash, acc);
		sync();
	}
}
// todo -- should do all database scan incase hash conflict
void account_database::store(const hash_digest& hash, const account account)
{
    const auto& name = account.get_name();
    const auto account_data = account.to_data();

    const auto check_store = [this, &name, &hash, &account_data]() {
        auto&& result = get_account_result(hash);
        if (!result) {
            return true;
        }

        auto detail = result.get_account_detail();
        if (!detail) {
            return true;
        }

        // account exist -- check duplicate
        if (detail->to_data() == account_data) {
            // if completely same, no need to store the same data.
            return false;
        }

        // account exist -- check hash conflict
        if (detail->get_name() != name) {
            log::error("account_database")
                << detail->get_name()
                << " is already exist and has same hash "
                << encode_hash(hash) << " with this name "
                << name;
            // for security reason, don't store account with hash conflict.
            return false;
        }

        // account exist -- remove old value
        remove(hash);
        sync();
        return true;
    };

    if (check_store()) {
        // actually store account
        const auto acc_size = account.serialized_size();
        BITCOIN_ASSERT(acc_size <= max_size_t);
        const auto value_size = static_cast<size_t>(acc_size);

        auto write = [&account_data](memory_ptr data)
        {
            auto serial = make_serializer(REMAP_ADDRESS(data));
            serial.write_data(account_data);
        };

        lookup_map_.store(hash, write, value_size);
    }

}

void account_database::recovery_account(const uint64_t & start, const uint64_t & offset)
{

	std::map<hash_digest, account> map_account;
	

	uint64_t cur = 0;
	//skip the head
	uint64_t header_size = slab_hash_table_header_size(9997)+8;

	uint64_t file_size = lookup_file_.size();

	if(start >= file_size)
		throw std::length_error("start position cannot be greater than file size");



	uint64_t cur_size  = std::max(header_size, start);
	uint64_t read_end_size = std::min(file_size, start+offset );

	while (cur_size < read_end_size)
	{
		try
		{
			memory_ptr ptr = lookup_file_.access();
			REMAP_INCREMENT(ptr, cur_size);

			const auto action = [&](memory_ptr elem) {

				const auto memory = REMAP_ADDRESS(elem);

				auto deserial = make_deserializer_unsafe(memory);
				hash_digest hash = deserial.read_hash();

				deserial.read_8_bytes_little_endian();

				account acc;
				if(!acc.from_data(deserial))
					return 0;


				auto itfind =  map_account.find(hash);
				if(itfind != map_account.end())
				{
					//if find hash in the map ,need check the name 
					if(itfind->second.get_name() != acc.get_name())
						return 0;

					itfind->second = acc;	
				}
				else
				{
					//if hash did not existed, need check name hash 								
					hash_digest name_hash = get_hash(acc.get_name());
					if(!std::equal(hash.begin(), hash.end(), name_hash.begin()))
						return 0;

					log::info("account_database")<<"find account:"<<acc.get_name();
					map_account[hash] = acc;					
				}
				
				return (int)acc.serialized_size();
			};

			int size = action(ptr);
			if (size == 0)
			{
				//if return size == 0,need add cur_size and continue traverse
				cur_size++;
			}
			else
			{
				cur_size += (size + sizeof(hash_digest) + 8);			
			}
			
			if ((cur % 1000) == 0)
			{
				auto reads = map_account.size();
				log::info("account_database") << reads << " accounts have been read.";
				log::info("account_database") << "=========" << cur_size << "/" << read_end_size << ", complete:" << cur_size * 100.0 / read_end_size << "%======";
			}

			cur++;			
		}
		catch (...)
		{
			cur_size++;
		}
	}


	log::info("account_database") <<" read account_table success! total accounts="<<map_account.size();

	path account_new = bc::default_data_path() / "account_table_new";

	if (!boost::filesystem::exists(account_new))
	{
		data_base::touch_file(account_new);
	}

	account_database account_db_(account_new);
	account_db_.create();
	account_db_.start();

	auto writes = 0;
	for(auto& acc : map_account)
	{
		account_db_.store(acc.first, acc.second);
		account_db_.sync();

		++writes;
		if ((writes % 1000) == 0)
		{
			log::info("account_database") << writes << " accounts have been written.";
			log::info("account_database") << "=========" << writes << "/" << map_account.size() << ", complete:" << writes * 100.0 / map_account.size() << "%======";	
		}
	}
	account_db_.close();

	log::info("account_database") <<" accounts recovery success!";

}

//memory_ptr base_database::get(const hash_digest& hash) const
std::shared_ptr<std::vector<account>> account_database::get_accounts() const
{
	auto vec_acc = std::make_shared<std::vector<account>>();
	uint64_t i = 0;
	for( i = 0; i < number_buckets; i++ ) {
	    auto memo = lookup_map_.find(i);
		//log::debug("get_accounts size=")<<memo->size();
		if(memo->size()) 
		{			
			const auto action = [&](memory_ptr elem)
			{
				const auto memory = REMAP_ADDRESS(elem);
				auto deserial = make_deserializer_unsafe(memory);
				vec_acc->push_back(account::factory_from_data(deserial));				
			};
			std::for_each(memo->begin(), memo->end(), action);
		}
	}
	return vec_acc;
}




// copy from block_chain_imp.cpp
inline hash_digest account_database::get_hash(const std::string& str)
{
	data_chunk data(str.begin(), str.end());
	return sha256_hash(data); 
}

account_result account_database::get_account_result(const hash_digest& hash) const
{
	const auto memory = get(hash);
    return account_result(memory);
}

} // namespace database
} // namespace libbitcoin
