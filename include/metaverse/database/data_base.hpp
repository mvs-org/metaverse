/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
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
#ifndef MVS_DATABASE_DATA_BASE_HPP
#define MVS_DATABASE_DATA_BASE_HPP

#include <atomic>
#include <cstddef>
#include <memory>
#include <boost/filesystem.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/databases/block_database.hpp>
#include <metaverse/database/databases/spend_database.hpp>
#include <metaverse/database/databases/transaction_database.hpp>
#include <metaverse/database/databases/history_database.hpp>
#include <metaverse/database/databases/stealth_database.hpp>
#include <metaverse/database/define.hpp>
#include <metaverse/database/settings.hpp>

#include <boost/variant.hpp>
#include <metaverse/bitcoin/chain/attachment/asset/asset.hpp>
#include <metaverse/bitcoin/chain/attachment/etp/etp.hpp>
#include <metaverse/bitcoin/chain/attachment/message/message.hpp>
#include <metaverse/bitcoin/chain/attachment/account/account.hpp>
#include <metaverse/bitcoin/chain/attachment/asset/asset_detail.hpp>
#include <metaverse/bitcoin/chain/attachment/asset/asset_transfer.hpp>
#include <metaverse/bitcoin/chain/attachment/attachment.hpp>

#include <metaverse/database/databases/account_database.hpp>
#include <metaverse/database/databases/account_address_database.hpp>
#include <metaverse/database/databases/asset_database.hpp>
#include <metaverse/database/databases/blockchain_asset_database.hpp>
#include <metaverse/database/databases/address_asset_database.hpp>
#include <metaverse/database/databases/account_asset_database.hpp>
#include <metaverse/bitcoin/chain/attachment/did/did.hpp>
#include <metaverse/database/databases/blockchain_asset_cert_database.hpp>
#include <metaverse/database/databases/blockchain_did_database.hpp>
#include <metaverse/database/databases/address_did_database.hpp>

using namespace libbitcoin::wallet;
using namespace libbitcoin::chain;

namespace libbitcoin {
namespace database {

typedef uint64_t handle;

class BCD_API data_base
{
public:
    typedef boost::filesystem::path path;

    class store
    {
    public:
        store(const path& prefix);
        bool touch_all() const;
        bool touch_dids() const;
        bool dids_exist() const;

        path database_lock;
        path blocks_lookup;
        path blocks_index;
        path history_lookup;
        path history_rows;
        path stealth_rows;
        path spends_lookup;
        path transactions_lookup;
		/* begin database for account, asset, address_asset, did relationship */
        path accounts_lookup;
        path assets_lookup;
        path certs_lookup;
        path address_assets_lookup;
        path address_assets_rows;
        path account_assets_lookup;
        path account_assets_rows;
        path account_dids_lookup;
        path account_dids_rows;
        path dids_lookup;
        path address_dids_lookup;
        path address_dids_rows;
        path account_addresses_lookup;
        path account_addresses_rows;
		/* end database for account, asset, address_asset, did ,address_did relationship */
    };


    class blockchain_store
    {
    public:
        blockchain_store(const path& prefix);
        bool touch_all() const;

        path database_lock;
        path blocks_lookup;
        path blocks_index;
        path history_lookup;
        path history_rows;
        path stealth_rows;
        path spends_lookup;
        path transactions_lookup;
		/* begin database for account, asset, address_asset, did relationship */
        path assets_lookup;
        path certs_lookup;
        path address_assets_lookup;
        path address_assets_rows;
        path dids_lookup;
        path address_dids_lookup;
        path address_dids_rows;
		/* end database for account, asset, address_asset, did relationship */
    };

    class blockchain_asset_store
    {
    public:
        blockchain_asset_store(const path& prefix);
        bool touch_all() const;
		/* begin database for account, asset, address_asset relationship */
        path assets_lookup;
		/* end database for account, asset, address_asset relationship */
    };

    class blockchain_did_store
    {
    public:
        blockchain_did_store(const path& prefix);
        bool touch_all() const;
		/* begin database for account, did, address_did relationship */
        path dids_lookup;
		/* end database for account, did, address_did relationship */
    };

	class db_metadata
	{
	public:
		db_metadata();
		db_metadata(std::string version);
		void reset();
		bool from_data(const data_chunk& data);
		bool from_data(std::istream& stream);
		bool from_data(reader& source);
		data_chunk to_data() const;
		void to_data(std::ostream& stream) const;
		void to_data(writer& sink) const;
		uint64_t serialized_size() const;

#ifdef MVS_DEBUG
		std::string to_string() const;
#endif
		friend std::istream& operator>>(std::istream& input, db_metadata& metadata);
		friend std::ostream& operator<<(std::ostream& output, const db_metadata& metadata);
		static const std::string current_version;
		static const std::string file_name;

		std::string version_;
	};

    /// Create a new database file with a given path prefix and default paths.
    static bool initialize(const path& prefix, const chain::block& genesis);
    static bool initialize_dids(const path& prefix);
    static bool touch_file(const path& file_path);
	static void write_metadata(const path& metadata_path, data_base::db_metadata& metadata);
	static void read_metadata(const path& metadata_path, data_base::db_metadata& metadata);
    /// Construct all databases.
    data_base(const settings& settings);

    /// Stop all databases (threads must be joined).
    ~data_base();
	bool clear_block_db();
    // Startup and shutdown.
    // ------------------------------------------------------------------------

    /// Create and start all databases.
    bool create();
    bool create_dids();
	bool blockchain_create();
	bool blockchain_asset_create();
	bool blockchain_did_create();

	bool account_db_start();
    /// Start all databases.
    bool start();

    /// Signal all databases to stop work.
    bool stop();

    /// Stop all databases (threads must be joined).
    bool close();

    // Locking.
    // ------------------------------------------------------------------------

    handle begin_read();
    bool begin_write();
    bool end_write();
    bool is_read_valid(handle handle);
    bool is_write_locked(handle handle);

    // Push and pop.
    // ------------------------------------------------------------------------

    /// Commit block at next height with indexing and no duplicate protection.
    void push(const chain::block& block);

    /// Commit block at given height with indexing and no duplicate protection.
    /// If height is not count + 1 then the count will not equal top height.
    void push(const chain::block& block, uint64_t height);

    /// Throws if the chain is empty.
    chain::block pop();

    /* begin store asset info into  database */

	void push_attachemnt(const attachment& attach, const payment_address& address,
			const output_point& outpoint, uint32_t output_height, uint64_t value);

	void push_etp(const etp& etp, const short_hash& key,
			const output_point& outpoint, uint32_t output_height, uint64_t value);

	void push_etp_award(const etp_award& etp, const short_hash& key,
			const output_point& outpoint, uint32_t output_height, uint64_t value);

	void push_message(const chain::blockchain_message& msg, const short_hash& key,
			const output_point& outpoint, uint32_t output_height, uint64_t value);

	void push_asset(const asset& sp, const short_hash& key,
				const output_point& outpoint, uint32_t output_height, uint64_t value);

	void push_asset_cert(const asset_cert& sp_cert, const short_hash& key,
				const output_point& outpoint, uint32_t output_height, uint64_t value);

	void push_asset_detail(const asset_detail& sp_detail, const short_hash& key,
				const output_point& outpoint, uint32_t output_height, uint64_t value);

	void push_asset_transfer(const asset_transfer& sp_transfer, const short_hash& key,
				const output_point& outpoint, uint32_t output_height, uint64_t value);

    void push_did(const did& sp, const short_hash& key,
				const output_point& outpoint, uint32_t output_height, uint64_t value);

    void push_did_detail(const did_detail& sp_detail, const short_hash& key,
				const output_point& outpoint, uint32_t output_height, uint64_t value);

   class attachment_visitor : public boost::static_visitor<void>
	{
	public:
		attachment_visitor(data_base* db, const short_hash& sh_hash,  const output_point& outpoint,
			uint32_t output_height, uint64_t value):
			db_(db), sh_hash_(sh_hash), outpoint_(outpoint), output_height_(output_height), value_(value)
		{

		}
		void operator()(const asset &t) const
		{
			return db_->push_asset(t, sh_hash_, outpoint_, output_height_, value_);
		}
		void operator()(const asset_cert &t) const
		{
			return db_->push_asset_cert(t, sh_hash_, outpoint_, output_height_, value_);
		}
		void operator()(const etp &t) const
		{
			return db_->push_etp(t, sh_hash_, outpoint_, output_height_, value_);
		}
		void operator()(const etp_award &t) const
		{
			return db_->push_etp_award(t, sh_hash_, outpoint_, output_height_, value_);
		}
		void operator()(const chain::blockchain_message &t) const
		{
			return db_->push_message(t, sh_hash_, outpoint_, output_height_, value_);
		}
        void operator()(const did &t) const
		{
			return db_->push_did(t, sh_hash_, outpoint_, output_height_, value_);
		}
	private:
        data_base* db_;
		short_hash sh_hash_;
		output_point outpoint_;
		uint32_t output_height_;
		uint64_t value_;
	};

	class asset_visitor : public boost::static_visitor<void>
	{
	public:
		asset_visitor(data_base* db, const short_hash& key,
			const output_point& outpoint, uint32_t output_height, uint64_t value):
			db_(db), key_(key), outpoint_(outpoint), output_height_(output_height), value_(value)
		{

		}
		void operator()(const asset_detail &t) const
		{
			return db_->push_asset_detail(t, key_, outpoint_, output_height_, value_);
		}
		void operator()(const asset_transfer &t) const
		{
		 	return db_->push_asset_transfer(t, key_, outpoint_, output_height_, value_);
		}
	private:
        data_base* db_;
		short_hash key_;
		output_point outpoint_;
		uint32_t output_height_;
		uint64_t value_;
	};

    class did_visitor : public boost::static_visitor<void>
	{
	public:
		did_visitor(data_base* db, const short_hash& key,
			const output_point& outpoint, uint32_t output_height, uint64_t value):
			db_(db), key_(key), outpoint_(outpoint), output_height_(output_height), value_(value)
		{

		}
		void operator()(const did_detail &t) const
		{
			return db_->push_did_detail(t, key_, outpoint_, output_height_, value_);
		}

	private:
        data_base* db_;
		short_hash key_;
		output_point outpoint_;
		uint32_t output_height_;
		uint64_t value_;
	};
	void set_admin(const std::string& name, const std::string& passwd);
   /* begin store asset info into  database */

protected:
    data_base(const store& paths, size_t history_height, size_t stealth_height);
    data_base(const path& prefix, size_t history_height, size_t stealth_height);

private:
    typedef chain::input::list inputs;
    typedef chain::output::list outputs;
    typedef std::atomic<size_t> sequential_lock;
    typedef boost::interprocess::file_lock file_lock;

    static void uninitialize_lock(const path& lock);
    static file_lock initialize_lock(const path& lock);

    void synchronize();
    void synchronize_dids();
    void push_inputs(const hash_digest& tx_hash, size_t height,
        const inputs& inputs);
    void push_outputs(const hash_digest& tx_hash, size_t height,
        const outputs& outputs);
    void push_stealth(const hash_digest& tx_hash, size_t height,
        const outputs& outputs);
    void pop_inputs(const inputs& inputs, size_t height);
    void pop_outputs(const outputs& outputs, size_t height);

    const path lock_file_path_;
    const size_t history_height_;
    const size_t stealth_height_;

    // Atomic counter for implementing the sequential lock pattern.
    sequential_lock sequential_lock_;

    // Allows us to restrict database access to our process (or fail).
    std::shared_ptr<file_lock> file_lock_;

    // Cross-database mutext to prevent concurrent file remapping.
    std::shared_ptr<shared_mutex> mutex_;

	// temp block timestamp
	uint32_t timestamp_;

public:

    /// Individual database query engines.
    block_database blocks;
    history_database history;
    spend_database spends;
    stealth_database stealth;
    transaction_database transactions;
	/* begin database for account, asset, address_asset,did relationship */
    account_database accounts;
    //asset_database assets;
    blockchain_asset_database assets;
    address_asset_database address_assets;
    account_asset_database account_assets;
    blockchain_asset_cert_database certs;
    //did_database dids;
    blockchain_did_database dids;
    address_did_database address_dids;
    account_address_database account_addresses;
	/* end database for account, asset, address_asset relationship */
};

} // namespace database
} // namespace libbitcoin

#endif

