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
#ifndef LIBBITCOIN_NODE_PROTOCOL_BLOCK_IN_HPP
#define LIBBITCOIN_NODE_PROTOCOL_BLOCK_IN_HPP

#include <atomic>
#include <cstddef>
#include <memory>
#include <bitcoin/blockchain.hpp>
#include <bitcoin/network.hpp>
#include <bitcoin/node/define.hpp>

namespace libbitcoin {
namespace node {

class BCN_API protocol_block_in
  : public network::protocol_timer, track<protocol_block_in>
{
public:
    typedef std::shared_ptr<protocol_block_in> ptr;

    /// Construct a block protocol instance.
    protocol_block_in(network::p2p& network, network::channel::ptr channel,
        blockchain::block_chain& blockchain);

    /// Start the protocol.
    virtual void start();

private:
    // Local type aliases.
    typedef message::get_data::ptr get_data_ptr;
    typedef message::block_message::ptr block_ptr;
    typedef message::headers::ptr headers_ptr;
    typedef message::inventory::ptr inventory_ptr;
    typedef message::not_found::ptr not_found_ptr;
    typedef message::block_message::ptr_list block_ptr_list;

    void get_block_inventory(const code& ec);
    void send_get_blocks(const hash_digest& stop_hash);
    void send_get_data(const code& ec, get_data_ptr message);

    bool handle_receive_block(const code& ec, block_ptr message);
    bool handle_receive_headers(const code& ec, headers_ptr message);
    bool handle_receive_inventory(const code& ec, inventory_ptr message);
    bool handle_receive_not_found(const code& ec, not_found_ptr message);
    void handle_filter_orphans(const code& ec, get_data_ptr message);
    void handle_store_block(const code& ec, block_ptr message);
    void handle_fetch_block_locator(const code& ec, const hash_list& locator,
        const hash_digest& stop_hash);
    bool handle_reorganized(const code& ec, size_t fork_point,
        const block_ptr_list& incoming, const block_ptr_list& outgoing);

    blockchain::block_chain& blockchain_;
    bc::atomic<hash_digest> last_locator_top_;
    bc::atomic<hash_digest> current_chain_top_;
    const bool headers_from_peer_;
};

} // namespace node
} // namespace libbitcoin

#endif
