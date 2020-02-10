/**
 * Copyright (c) 2011-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2020 metaverse core developers (see MVS-AUTHORS)
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
#ifndef MVS_NODE_PROTOCOL_BLOCK_SYNC_HPP
#define MVS_NODE_PROTOCOL_BLOCK_SYNC_HPP

#include <cstddef>
#include <memory>
#include <metaverse/blockchain.hpp>
#include <metaverse/network.hpp>
#include <metaverse/node/define.hpp>
#include <metaverse/node/utility/reservation.hpp>

namespace libbitcoin {
namespace node {

/// Blocks sync protocol, thread safe.
class BCN_API protocol_block_sync
  : public network::protocol_timer, public track<protocol_block_sync>
{
public:
    typedef std::shared_ptr<protocol_block_sync> ptr;

    /// Construct a block sync protocol instance.
    protocol_block_sync(network::p2p& network, network::channel::ptr channel,
        reservation::ptr row);

    /// Start the protocol.
    virtual void start(event_handler handler);

private:
    typedef message::block_message::ptr block_ptr;

    void send_get_blocks(event_handler complete, bool reset);
    void handle_send(const code& ec, event_handler complete);
    void handle_event(const code& ec, event_handler complete);
    void blocks_complete(const code& ec, event_handler handler);
    bool handle_receive(const code& ec, block_ptr message,
        event_handler complete);

    reservation::ptr reservation_;
};

} // namespace node
} // namespace libbitcoin

#endif
