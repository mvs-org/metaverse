/**
 * Copyright (c) 2011-2015 metaverse developers (see AUTHORS)
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
#ifndef MVS_DATABASE_ACCOUNT_DID_DATABASE_HPP
#define MVS_DATABASE_ACCOUNT_DID_DATABASE_HPP

#include <memory>
#include <boost/filesystem.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/define.hpp>
#include <metaverse/database/memory/memory_map.hpp>
#include <metaverse/database/primitives/record_multimap.hpp>
#include <metaverse/bitcoin/chain/attachment/did/did_detail.hpp>
#include <metaverse/bitcoin/chain/business_data.hpp>

using namespace libbitcoin::chain;

namespace libbitcoin {
namespace database {

struct BCD_API account_did_statinfo
{
    /// Number of buckets used in the hashtable.
    /// load factor = addrs / buckets
    const size_t buckets;

    /// Total number of unique addresses in the database.
    const size_t addrs;

    /// Total number of rows across all addresses.
    const size_t rows;
};

/// This is a multimap where the key is the Bitcoin address hash,
/// which returns several rows giving the account_did for that address.
class BCD_API account_did_database
{
public:
    /// Construct the database.
    account_did_database(const boost::filesystem::path& lookup_filename,
        const boost::filesystem::path& rows_filename,
        std::shared_ptr<shared_mutex> mutex=nullptr);

    /// Close the database (all threads must first be stopped).
    ~account_did_database();

    /// Initialize a new account_did database.
    bool create();

    /// Call before using the database.
    bool start();

    /// Call to signal a stop of current operations.
    bool stop();

    /// Call to unload the memory map.
    bool close();
	
	void store(const short_hash& key, const did_detail& account_did);
	
	void delete_last_row(const short_hash& key);
	
	did_detail::list get(const short_hash& key) const;
	
	std::shared_ptr<did_detail> get(const short_hash& key, const std::string& address) const;
	
	/// get dids whose status is not issued and stored in local database (not in blockchain)
	std::shared_ptr<std::vector<business_address_did>> get_unissued_dids(const short_hash& key) const;
	
    /// Synchonise with disk.
    void sync();

    /// Return statistical info about the database.
    account_did_statinfo statinfo() const;

private:
    typedef record_hash_table<short_hash> record_map;
    typedef record_multimap<short_hash> record_multiple_map;

    /// Hash table used for start index lookup for linked list by address hash.
    memory_map lookup_file_;
    record_hash_table_header lookup_header_;
    record_manager lookup_manager_;
    record_map lookup_map_;

    /// List of account_did rows.
    memory_map rows_file_;
    record_manager rows_manager_;
    record_list rows_list_;
    record_multiple_map rows_multimap_;
};

} // namespace database
} // namespace libbitcoin

#endif


