/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin-node.
 *
 * libbitcoin-node is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_BLOCKCHAIN_TRANSACTION_POOL_INDEX_HPP
#define LIBBITCOIN_BLOCKCHAIN_TRANSACTION_POOL_INDEX_HPP

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/blockchain/define.hpp>
#include <bitcoin/blockchain/block_chain.hpp>

namespace libbitcoin {
namespace blockchain {

/// This class is thread safe.
class BCB_API transaction_pool_index
{
public:
    typedef handle0 completion_handler;
    typedef handle2<chain::spend_info::list, chain::output_info::list>
        query_handler;
    typedef block_chain::history_fetch_handler fetch_handler;

    transaction_pool_index(threadpool& pool, block_chain& blockchain);

    /// This class is not copyable.
    transaction_pool_index(const transaction_pool_index&) = delete;
    void operator=(const transaction_pool_index&) = delete;

    /// Start the transaction pool.
    void start();

    /// Signal stop of current work, speeds shutdown.
    void stop();

    void fetch_all_history(const wallet::payment_address& address,
        size_t limit, size_t from_height, fetch_handler handler);

    void fetch_index_history(const wallet::payment_address& address,
        query_handler handler);

    void add(const chain::transaction& tx, completion_handler handler);
    void remove(const chain::transaction& tx, completion_handler handler);

private:
    typedef chain::spend_info spend_info;
    typedef chain::output_info output_info;
    typedef chain::history_compact::list history_list;
    typedef wallet::payment_address payment_address;
    typedef std::unordered_multimap<payment_address, spend_info> spends_map;
    typedef std::unordered_multimap<payment_address, output_info> outputs_map;

    static bool exists(history_list& history, const spend_info& spend);
    static bool exists(history_list& history, const output_info& output);
    static void add(history_list& history, const spend_info& spend);
    static void add(history_list& history, const output_info& output);
    static void add(history_list& history, const spend_info::list& spends);
    static void add(history_list& history, const output_info::list& outputs);
    static void index_history_fetched(const code& ec,
        const spend_info::list& spends, const output_info::list& outputs,
        const history_list& history, fetch_handler handler);

    void blockchain_history_fetched(const code& ec,
        const history_list& history, const wallet::payment_address& address,
        fetch_handler handler);

    void do_add(const chain::transaction& tx, completion_handler handler);
    void do_remove(const chain::transaction& tx, completion_handler handler);
    void do_fetch(const wallet::payment_address& payaddr,
        query_handler handler);

    // This is protected by mutex.
    spends_map spends_map_;
    upgrade_mutex spends_mutex_;

    // This is protected by mutex.
    outputs_map outputs_map_;
    upgrade_mutex outputs_mutex_;

    // These are thread safe.
    dispatcher dispatch_;
    block_chain& blockchain_;
    std::atomic<bool> stopped_;
};

} // namespace blockchain
} // namespace libbitcoin

#endif
