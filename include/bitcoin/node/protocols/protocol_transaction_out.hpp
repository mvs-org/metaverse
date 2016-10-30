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
#ifndef LIBBITCOIN_NODE_PROTOCOL_TRANSACTION_OUT_HPP
#define LIBBITCOIN_NODE_PROTOCOL_TRANSACTION_OUT_HPP

#include <atomic>
#include <cstdint>
#include <memory>
#include <bitcoin/blockchain.hpp>
#include <bitcoin/network.hpp>
#include <bitcoin/node/define.hpp>

namespace libbitcoin {
namespace node {

class BCN_API protocol_transaction_out
  : public network::protocol_events, track<protocol_transaction_out>
{
public:
    typedef std::shared_ptr<protocol_transaction_out> ptr;

    /// Construct a transaction protocol instance.
    protocol_transaction_out(network::p2p& network,
        network::channel::ptr channel, blockchain::block_chain& blockchain,
        blockchain::transaction_pool& pool);

    /// Start the protocol.
    virtual void start();

private:
    // Local type aliases.
    typedef message::transaction_message::ptr transaction_ptr;
    typedef message::fee_filter::ptr fee_filter_ptr;
    typedef message::memory_pool::ptr memory_pool_ptr;
    typedef message::get_data::ptr get_data_ptr;
    typedef chain::point::indexes index_list;

    void send_transaction(const code& ec,
        const chain::transaction& transaction, const hash_digest& hash);

    bool handle_receive_get_data(const code& ec, get_data_ptr message);
    bool handle_receive_fee_filter(const code& ec, fee_filter_ptr message);
    void handle_receive_memory_pool(const code& ec, memory_pool_ptr message);

    void handle_stop(const code&);
    bool handle_floated(const code& ec, const index_list& unconfirmed,
        transaction_ptr message);

    blockchain::block_chain& blockchain_;
    blockchain::transaction_pool& pool_;
    std::atomic<uint64_t> minimum_fee_;
    const bool relay_to_peer_;
};

} // namespace node
} // namespace libbitcoin

#endif
