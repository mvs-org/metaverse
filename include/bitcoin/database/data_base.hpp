/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
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
#ifndef LIBBITCOIN_DATABASE_DATA_BASE_HPP
#define LIBBITCOIN_DATABASE_DATA_BASE_HPP

#include <atomic>
#include <cstddef>
#include <memory>
#include <boost/filesystem.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/database/databases/block_database.hpp>
#include <bitcoin/database/databases/spend_database.hpp>
#include <bitcoin/database/databases/transaction_database.hpp>
#include <bitcoin/database/databases/history_database.hpp>
#include <bitcoin/database/databases/stealth_database.hpp>
#include <bitcoin/database/define.hpp>
#include <bitcoin/database/settings.hpp>

#include <boost/variant.hpp>
#include <bitcoin/bitcoin/chain/attachment/asset/asset.hpp>
#include <bitcoin/bitcoin/chain/attachment/etp/etp.hpp>
#include <bitcoin/bitcoin/chain/attachment/account/account.hpp>
#include <bitcoin/bitcoin/chain/attachment/asset/asset_detail.hpp>
#include <bitcoin/bitcoin/chain/attachment/asset/asset_transfer.hpp>
#include <bitcoin/bitcoin/chain/attachment/attachment.hpp>

#include <bitcoin/database/databases/account_database.hpp>
#include <bitcoin/database/databases/account_address_database.hpp>
#include <bitcoin/database/databases/asset_database.hpp>
#include <bitcoin/database/databases/account_asset_database.hpp>
//#include <bitcoin/bitcoin/wallet/payment_address.hpp>

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

        path database_lock;
        path blocks_lookup;
        path blocks_index;
        path history_lookup;
        path history_rows;
        path stealth_rows;
        path spends_lookup;
        path transactions_lookup;
		/* begin database for account, asset, account_asset relationship */
        path accounts_lookup;
        path assets_lookup;
        path account_assets_lookup;
        path account_addresses_lookup;
		/* end database for account, asset, account_asset relationship */
    };

    /// Create a new database file with a given path prefix and default paths.
    static bool initialize(const path& prefix, const chain::block& genesis);
    static bool touch_file(const path& file_path);

    /// Construct all databases.
    data_base(const settings& settings);

    /// Stop all databases (threads must be joined).
    ~data_base();

    // Startup and shutdown.
    // ------------------------------------------------------------------------

    /// Create and start all databases.
    bool create();

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

    void process_attachemnt(attachment& attach, payment_address& address);
    void process_asset(asset& sp, payment_address address);
    void push_asset_detail(asset_detail& sp_detail);
    void push_asset_transfer(asset_transfer& sp_transfer, payment_address& address);

   class attachment_visitor : public boost::static_visitor<void>
	{
	public:
		attachment_visitor(payment_address& address,  
                data_base* db):address_(address),db_(db)
		{

		}
		void operator()(asset &t)
		{
			return db_->process_asset(t, address_);
		}
		void operator()(etp &t)
		{
			return ;
		}
		payment_address& address_;
        data_base* db_;
	};

	class asset_visitor : public boost::static_visitor<void>
	{
	public:
		asset_visitor(payment_address& address,
                data_base* db):address_(address),db_(db)
		{

		}
		void operator()(asset_detail &t)
		{
			return db_->push_asset_detail(t);
		}
		void operator()(asset_transfer &t)
		{
		 	return db_->push_asset_transfer(t, address_);
		}
		payment_address& address_;
        data_base* db_;
	};

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

public:

    /// Individual database query engines.
    block_database blocks;
    history_database history;
    spend_database spends;
    stealth_database stealth;
    transaction_database transactions;
	/* begin database for account, asset, account_asset relationship */
    account_database accounts;
    asset_database assets;
    account_asset_database account_assets;
    account_address_database account_addresses;
	/* end database for account, asset, account_asset relationship */
};

} // namespace database
} // namespace libbitcoin

#endif
