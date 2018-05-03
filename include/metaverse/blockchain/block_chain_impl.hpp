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
#ifndef MVS_BLOCKCHAIN_BLOCK_CHAIN_IMPL_HPP
#define MVS_BLOCKCHAIN_BLOCK_CHAIN_IMPL_HPP

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <functional>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database.hpp>
#include <metaverse/blockchain/block_chain.hpp>
#include <metaverse/blockchain/define.hpp>
#include <metaverse/blockchain/organizer.hpp>
#include <metaverse/blockchain/settings.hpp>
#include <metaverse/blockchain/simple_chain.hpp>
#include <metaverse/blockchain/transaction_pool.hpp>
#include <metaverse/bitcoin/chain/header.hpp>
#include <metaverse/bitcoin/chain/header.hpp>

#define  LOG_BLOCK_CHAIN_IMPL  "block_chain_impl"
using namespace libbitcoin::message;

namespace libbitcoin {
namespace blockchain {

typedef console_result operation_result;

/// The simple_chain interface portion of this class is not thread safe.
class BCB_API block_chain_impl
  : public block_chain, public simple_chain
{
public:
    block_chain_impl(threadpool& pool,
        const blockchain::settings& chain_settings,
        const database::settings& database_settings);

    /// The database is closed on destruct, threads must be joined.
    ~block_chain_impl();

    /// This class is not copyable.
    block_chain_impl(const block_chain_impl&) = delete;
    void operator=(const block_chain_impl&) = delete;

    // Properties (thread safe).
    // ------------------------------------------------------------------------

    // Get a reference to the transaction pool.
    transaction_pool& pool();

    // Get a reference to the blockchain configuration settings.
    const settings& chain_settings() const;

    // block_chain start/stop (thread safe).
    // ------------------------------------------------------------------------

    /// Start or restart the blockchain.
    virtual bool start();

    /// Signal stop of current work, speeds shutdown with multiple threads.
    virtual bool stop();

    /// Close the blockchain, threads must first be joined, can be restarted.
    virtual bool close();

    // simple_chain (NOT THREAD SAFE).
    // ------------------------------------------------------------------------

    /// Return the first and last gaps in the blockchain, or false if none.
    bool get_gap_range(uint64_t& out_first, uint64_t& out_last) const;

    /// Return the next chain gap at or after the specified start height.
    bool get_next_gap(uint64_t& out_height, uint64_t start_height) const;

    /// Get the dificulty of a block at the given height.
    bool get_difficulty(u256& out_difficulty, uint64_t height) const;

    /// Get the header of the block at the given height.
    bool get_header(chain::header& out_header, uint64_t height) const;

    /// Get the height of the block with the given hash.
    bool get_height(uint64_t& out_height, const hash_digest& block_hash) const;

    /// Get height of latest block.
    bool get_last_height(uint64_t& out_height) const;

    /// Get the hash digest of the transaction of the outpoint.
    bool get_outpoint_transaction(hash_digest& out_transaction,
        const chain::output_point& outpoint) const;

    /// Get the transaction of the given hash and its block height.
    bool get_transaction(chain::transaction& out_transaction,
        uint64_t& out_block_height, const hash_digest& transaction_hash) const;

    /// Import a block to the blockchain.
    bool import(chain::block::ptr block, uint64_t height);

    /// Append the block to the top of the chain.
    bool push(block_detail::ptr block);

    /// Remove blocks at or above the given height, returning them in order.
    bool pop_from(block_detail::list& out_blocks, uint64_t height);

    // block_chain queries (thread safe).
    // ------------------------------------------------------------------------

    /// Store a block to the blockchain, with indexing and validation.
    void store(message::block_message::ptr block,
        block_store_handler handler);

    /// fetch a block by height.
    void fetch_block(uint64_t height, block_fetch_handler handler);

    /// fetch a block by height.
    void fetch_block(const hash_digest& hash, block_fetch_handler handler);

    /// fetch block header by height.
    void fetch_block_header(uint64_t height,
        block_header_fetch_handler handler);

    /// fetch block header by hash.
    void fetch_block_header(const hash_digest& hash,
        block_header_fetch_handler handler);

    /// fetch a merkle block by height.
    void fetch_merkle_block(uint64_t height,
        merkle_block_fetch_handler handler);

    /// fetch a merkle block by height.
    void fetch_merkle_block(const hash_digest& hash,
        merkle_block_fetch_handler handler);

    /// fetch hashes of transactions for a block, by block height.
    void fetch_block_transaction_hashes(uint64_t height,
        transaction_hashes_fetch_handler handler);

    /// fetch hashes of transactions for a block, by block hash.
    void fetch_block_transaction_hashes(const hash_digest& hash,
        transaction_hashes_fetch_handler handler);

    /// fetch a block locator relative to the current top and threshold.
    void fetch_block_locator(block_locator_fetch_handler handler);

    /// fetch the set of block hashes indicated by the block locator.
    void fetch_locator_block_hashes(const message::get_blocks& locator,
        const hash_digest& threshold, size_t limit,
        locator_block_hashes_fetch_handler handler);

    /// fetch the set of block headers indicated by the block locator.
    void fetch_locator_block_headers(const message::get_headers& locator,
        const hash_digest& threshold, size_t limit,
        locator_block_headers_fetch_handler handler);

    /// fetch height of block by hash.
    void fetch_block_height(const hash_digest& hash,
        block_height_fetch_handler handler);

    /// fetch height of latest block.
    void fetch_last_height(last_height_fetch_handler handler);

    /// fetch transaction by hash.
    void fetch_transaction(const hash_digest& hash,
        transaction_fetch_handler handler);

    /// fetch height and offset within block of transaction by hash.
    void fetch_transaction_index(const hash_digest& hash,
        transaction_index_fetch_handler handler);

    /// fetch spend of an output point.
    void fetch_spend(const chain::output_point& outpoint,
        spend_fetch_handler handler);

    /// fetch outputs, values and spends for an address.
    void fetch_history(const wallet::payment_address& address,
        uint64_t limit, uint64_t from_height, history_fetch_handler handler);

    bool fetch_history(const wallet::payment_address& address,
        uint64_t limit, uint64_t from_height, history_compact::list& history);

    history::list get_address_history(const wallet::payment_address& addr);

    /// fetch stealth results.
    void fetch_stealth(const binary& filter, uint64_t from_height,
        stealth_fetch_handler handler);

    /// filter out block hashes that exist in the store.
    virtual void filter_blocks(message::get_data::ptr message,
        result_handler handler);

    /// filter out block hashes that exist in the orphan pool.
    virtual void filter_orphans(message::get_data::ptr message,
        result_handler handler);

    /// filter out transaction hashes that exist in the store.
    virtual void filter_transactions(message::get_data::ptr message,
        result_handler handler);

    /// Subscribe to blockchain reorganizations.
    virtual void subscribe_reorganize(reorganize_handler handler);

	inline hash_digest get_hash(const std::string& str);
	inline short_hash get_short_hash(const std::string& str);

	// account related api
	std::shared_ptr<account> is_account_passwd_valid(const std::string& name, const std::string& passwd);
	void set_account_passwd(const std::string& name, const std::string& passwd);
	bool is_account_exist(const std::string& name);
	bool is_admin_account(const std::string& name);
	operation_result store_account(std::shared_ptr<account> acc);
	std::shared_ptr<account> get_account(const std::string& name);
	std::shared_ptr<std::vector<account>> get_accounts();
	account_status get_account_user_status(const std::string& name);
	account_status get_account_system_status(const std::string& name);
	bool set_account_user_status(const std::string& name, uint8_t status);
	bool set_account_system_status(const std::string& name, uint8_t status);
	operation_result delete_account(const std::string& name);
	operation_result delete_account_address(const std::string& name);

	std::shared_ptr<std::vector<business_history>> get_address_business_history(const std::string& addr,
					business_kind kind, uint8_t confirmed);
	std::shared_ptr<std::vector<business_history>> get_address_business_history(const std::string& addr,
					business_kind kind, uint32_t time_begin, uint32_t time_end);
	std::shared_ptr<std::vector<business_history>> get_address_business_history(const std::string& addr);
	// account asset api
	operation_result store_account_asset(const asset_detail& detail);
	operation_result store_account_asset(std::shared_ptr<asset_detail> detail);
	operation_result delete_account_asset(const std::string& name);
	std::shared_ptr<std::vector<business_address_asset>> get_account_asset(const std::string& name,
			const std::string& asset_name, business_kind kind);
	std::shared_ptr<std::vector<business_address_asset>> get_account_asset(const std::string& name, const std::string& asset);
	std::shared_ptr<std::vector<business_address_asset>> get_account_assets(const std::string& name);
	std::shared_ptr<std::vector<business_address_asset>> get_account_assets(const std::string& name,
					business_kind kind);
    uint64_t get_address_asset_volume(const std::string& address, const std::string& asset);
    uint64_t get_account_asset_volume(const std::string& account, const std::string& asset);
    uint64_t get_asset_volume(const std::string& asset);
    asset_cert_type get_address_asset_cert_type(const std::string& address, const std::string& asset);
    std::shared_ptr<std::vector<business_address_asset_cert>> get_address_asset_certs(const std::string& address, const std::string& asset);
    std::shared_ptr<std::vector<business_address_asset_cert>> get_account_asset_certs(const std::string& account, const std::string& asset);

    std::shared_ptr<std::vector<asset_cert>> get_issued_asset_certs();
    bool is_asset_cert_exist(const std::string& symbol, asset_cert_type cert_mask);

	bool is_asset_exist(const std::string& asset_name, bool add_local_db=true);
	bool get_asset_height(const std::string& asset_name, uint64_t& height);
	std::shared_ptr<std::vector<asset_detail>> get_local_assets();
	std::shared_ptr<asset_detail> get_issued_asset(const std::string& symbol);
	std::shared_ptr<std::vector<business_address_asset>> get_account_assets();
	std::shared_ptr<std::vector<asset_detail>> get_issued_assets();
	std::shared_ptr<std::vector<business_address_asset>> get_account_unissued_assets(const std::string& name);
	std::shared_ptr<asset_detail> get_account_unissued_asset(const std::string& name,
		const std::string& symbol);
    //account did api
	bool is_did_exist(const std::string& did_name);
    bool get_did_height(const std::string& did_name, uint64_t& height);
    bool is_address_issued_did(const std::string& address);
    std::string get_did_from_address(const std::string& address);
	std::shared_ptr<did_detail> get_issued_did(std::string& symbol);
	std::shared_ptr<std::vector<did_detail>> get_issued_dids();

    //get history addresses from did symbol
    std::shared_ptr<std::vector<blockchain_did>>  get_did_history_addresses(const std::string & symbol);

	std::shared_ptr<std::vector<business_history>> get_account_business_history(const std::string& name,
					business_kind kind, uint32_t time_begin, uint32_t time_end);
	std::shared_ptr<std::vector<business_history>> get_address_business_history(const std::string& addr,
					const std::string& symbol, business_kind kind, uint8_t confirmed);
	std::shared_ptr<std::vector<business_record>> get_address_business_record(const std::string& addr,
			size_t from_height = 0, size_t limit = 0);
	std::shared_ptr<std::vector<business_record>> get_address_business_record(const std::string& addr,
					uint64_t start, uint64_t end, const std::string& symbol);
    std::shared_ptr<std::vector<business_record>> get_address_business_record(const std::string& address,
        const std::string& symbol, size_t start_height, size_t end_height, uint64_t limit, uint64_t page_number) const;
	std::shared_ptr<std::vector<account_address>> get_addresses();

	// account message api
	std::shared_ptr<std::vector<business_address_message>> get_account_messages(const std::string& name);

	// account adress related api
	operation_result store_account_address(std::shared_ptr<account_address> address);
	std::shared_ptr<account_address> get_account_address(const std::string& name, const std::string& address);
	std::shared_ptr<std::vector<account_address>> get_account_addresses(const std::string& name);
	void uppercase_symbol(std::string& symbol);

    bool is_valid_address(const std::string& address);
    bool is_payment_address(const std::string& address);
    bool is_stealth_address(const std::string& address);
    bool is_script_address(const std::string& address);
    bool is_blackhole_address(const std::string& address);

	void fired();
	organizer& get_organizer();
	bool get_transaction(const hash_digest& hash,
		chain::transaction& tx, uint64_t& tx_height);
	bool get_transaction_callback(const hash_digest& hash,
    std::function<void(const code&, const chain::transaction&)> handler);
	bool get_history_callback(const payment_address& address,
		size_t limit, size_t from_height,
		std::function<void(const code&, chain::history::list&)> handler);
	bool get_history(const wallet::payment_address& address,
		uint64_t limit, uint64_t from_height, history_compact::list& history);
	code validate_transaction(const chain::transaction& tx);
	code broadcast_transaction(const chain::transaction& tx);
    bool get_tx_inputs_etp_value (chain::transaction& tx, uint64_t& etp_val);
    void safe_store_account(account& acc, std::vector<std::shared_ptr<account_address>>& addresses);

    //return value:
    //0, success
    //-1, operate database fail
    //>0, validate error, return error block index
    int replace_chain(uint64_t begin_height, const block_detail::list& new_blocks, block_detail::list& released_blocks);

private:
    typedef std::function<bool(database::handle)> perform_read_functor;

    template <typename Handler, typename... Args>
    bool finish_fetch(database::handle handle, Handler handler, Args&&... args)
    {
        if (!database_.is_read_valid(handle))
            return false;

        handler(std::forward<Args>(args)...);
        return true;
    }

    template <typename Handler, typename... Args>
    void stop_write(Handler handler, Args&&... args)
    {
        const auto result = database_.end_write();
        BITCOIN_ASSERT(result);
        handler(std::forward<Args>(args)...);
    }

    void stop_write();
    void start_write();
    void do_store(message::block_message::ptr block,
        block_store_handler handler);

    ////void fetch_ordered(perform_read_functor perform_read);
    ////void fetch_parallel(perform_read_functor perform_read);
    void fetch_serial(perform_read_functor perform_read);
    bool stopped() const;

    std::atomic<bool> stopped_;
    const settings& settings_;

    // These are thread safe.
    organizer organizer_;
    ////dispatcher read_dispatch_;
    ////dispatcher write_dispatch_;
    blockchain::transaction_pool transaction_pool_;

    // This is protected by mutex.
    database::data_base database_;
    mutable shared_mutex mutex_;
};

} // namespace blockchain
} // namespace libbitcoin

#endif
