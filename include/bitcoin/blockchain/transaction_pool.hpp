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
#ifndef LIBBITCOIN_BLOCKCHAIN_TRANSACTION_POOL_HPP
#define LIBBITCOIN_BLOCKCHAIN_TRANSACTION_POOL_HPP

#include <atomic>
#include <cstddef>
#include <functional>
#include <boost/circular_buffer.hpp>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/blockchain/define.hpp>
#include <bitcoin/blockchain/block_chain.hpp>
#include <bitcoin/blockchain/settings.hpp>
#include <bitcoin/blockchain/transaction_pool_index.hpp>

namespace libbitcoin {
namespace blockchain {

/// This class is thread safe.
class BCB_API transaction_pool
{
public:
    typedef chain::point::indexes indexes;
    typedef message::get_data::ptr get_data_ptr;
    typedef message::transaction_message::ptr transaction_ptr;

    typedef handle0 result_handler;
    typedef handle1<transaction_ptr> fetch_handler;
    typedef handle1<transaction_ptr> confirm_handler;
    typedef handle2<transaction_ptr, indexes> validate_handler;
    typedef std::function<bool(const code&, const indexes&, transaction_ptr)>
        transaction_handler;
    typedef resubscriber<const code&, const indexes&, transaction_ptr>
        transaction_subscriber;

    static bool is_spent_by_tx(const chain::output_point& outpoint,
        const transaction_ptr tx);

    /// Construct a transaction memory pool.
    transaction_pool(threadpool& pool, block_chain& chain,
        const settings& settings);

    /// Clear the pool, threads must be joined.
    ~transaction_pool();

    /// This class is not copyable.
    transaction_pool(const transaction_pool&) = delete;
    void operator=(const transaction_pool&) = delete;

    /// Start the transaction pool.
    void start();

    /// Signal stop of current work, speeds shutdown.
    void stop();

    void inventory(message::inventory::ptr inventory);
    void fetch(const hash_digest& tx_hash, fetch_handler handler);
    void fetch_history(const wallet::payment_address& address, size_t limit,
        size_t from_height, block_chain::history_fetch_handler handler);
    void exists(const hash_digest& tx_hash, result_handler handler);
    void filter(get_data_ptr message, result_handler handler);
    void validate(transaction_ptr tx, validate_handler handler);
    void store(transaction_ptr tx, confirm_handler confirm_handler,
        validate_handler validate_handler);

    /// Subscribe to transaction acceptance into the mempool.
    void subscribe_transaction(transaction_handler handler);

protected:
    /// This is analogous to the orphan pool's block_detail.
    struct entry
    {
        transaction_ptr tx;
        confirm_handler handle_confirm;
    };

    typedef boost::circular_buffer<entry> buffer;
    typedef buffer::const_iterator const_iterator;

    typedef std::function<bool(const chain::input&)> input_compare;
    typedef message::block_message::ptr_list block_list;

    bool stopped();
    const_iterator find(const hash_digest& tx_hash) const;

    bool handle_reorganized(const code& ec, size_t fork_point,
        const block_list& new_blocks, const block_list& replaced_blocks);
    void handle_validated(const code& ec, transaction_ptr tx,
        const indexes& unconfirmed, validate_handler handler);

    void do_validate(transaction_ptr tx, validate_handler handler);
    void do_store(const code& ec, transaction_ptr tx,
        const indexes& unconfirmed, confirm_handler handle_confirm,
        validate_handler handle_validate);

    void notify_transaction(const chain::point::indexes& unconfirmed,
        transaction_ptr tx);

    void add(transaction_ptr tx, confirm_handler handler);
    void remove(const block_list& blocks);
    void clear(const code& ec);

    // These would be private but for test access.
    void delete_spent_in_blocks(const block_list& blocks);
    void delete_confirmed_in_blocks(const block_list& blocks);
    void delete_dependencies(const hash_digest& tx_hash, const code& ec);
    void delete_dependencies(const chain::output_point& point, const code& ec);
    void delete_dependencies(input_compare is_dependency, const code& ec);
    void delete_package(const code& ec);
    void delete_package(transaction_ptr tx, const code& ec);
    bool delete_single(const hash_digest& tx_hash, const code& ec);

    // The buffer is protected by non-concurrent dispatch.
    buffer buffer_;
    std::atomic<bool> stopped_;

private:
    // Unsafe methods limited to friend caller.
    friend class validate_transaction;

    // These methods are NOT thread safe.
    bool is_in_pool(const hash_digest& tx_hash) const;
    bool is_spent_in_pool(transaction_ptr tx) const;
    bool is_spent_in_pool(const chain::transaction& tx) const;
    bool is_spent_in_pool(const chain::output_point& outpoint) const;
    bool find(transaction_ptr& out_tx, const hash_digest& tx_hash) const;
    bool find(chain::transaction& out_tx, const hash_digest& tx_hash) const;

    // These are thread safe.
    dispatcher dispatch_;
    block_chain& blockchain_;
    transaction_pool_index index_;
    transaction_subscriber::ptr subscriber_;
    const bool maintain_consistency_;
};

} // namespace blockchain
} // namespace libbitcoin

#endif
