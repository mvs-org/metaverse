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
#include <bitcoin/node/protocols/protocol_block_sync.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <bitcoin/blockchain.hpp>
#include <bitcoin/network.hpp>
#include <bitcoin/node/utility/reservation.hpp>

namespace libbitcoin {
namespace node {

#define NAME "block_sync"
#define CLASS protocol_block_sync

using namespace bc::message;
using namespace bc::network;
using namespace std::placeholders;

// The interval in which block download rate is tested.
static const asio::seconds expiry_interval(5);

// Depends on protocol_header_sync, which requires protocol version 31800.
protocol_block_sync::protocol_block_sync(p2p& network, channel::ptr channel,
    reservation::ptr row)
  : protocol_timer(network, channel, true, NAME),
    reservation_(row),
    CONSTRUCT_TRACK(protocol_block_sync)
{
}

// Start sequence.
// ----------------------------------------------------------------------------

void protocol_block_sync::start(event_handler handler)
{
    auto complete = synchronize(BIND2(blocks_complete, _1, handler), 1, NAME);
    protocol_timer::start(expiry_interval, BIND2(handle_event, _1, complete));

    SUBSCRIBE3(block_message, handle_receive, _1, _2, complete);

    // This is the end of the start sequence.
    send_get_blocks(complete, true);
}

// Peer sync sequence.
// ----------------------------------------------------------------------------

void protocol_block_sync::send_get_blocks(event_handler complete, bool reset)
{
    if (stopped())
        return;

    if (reservation_->stopped())
    {
        log::debug(LOG_NODE)
            << "Stopping complete slot (" << reservation_->slot() << ").";
        complete(error::success);
        return;
    }

    // We may be a new channel (reset) or may have a new packet.
    const auto request = reservation_->request(reset);

    // Or we may be the same channel and with hashes already requested.
    if (request.inventories.empty())
        return;

    log::debug(LOG_NODE)
        << "Sending request of " << request.inventories.size()
        << " hashes for slot (" << reservation_->slot() << ").";

    SEND2(request, handle_send, _1, complete);
}

void protocol_block_sync::handle_send(const code& ec, event_handler complete)
{
    if (stopped())
        return;

    if (ec)
    {
        log::warning(LOG_NODE)
            << "Failure sending get blocks to slot (" << reservation_->slot()
            << ") " << ec.message();
        complete(ec);
    }
}

// The message subscriber implements an optimization to bypass queueing of
// block messages. This requires that this handler never call back into the
// subscriber. Otherwise a deadlock will result. This in turn requires that
// the 'complete' parameter handler never call into the message subscriber.
bool protocol_block_sync::handle_receive(const code& ec, block_ptr message,
    event_handler complete)
{
    if (stopped())
        return false;

    if (ec)
    {
        log::debug(LOG_NODE)
            << "Receive failure on slot (" << reservation_->slot() << ") "
            << ec.message();
        complete(ec);
        return false;
    }

    // Add the block to the blockchain store.
    reservation_->import(message);

    if (reservation_->toggle_partitioned())
    {
        log::debug(LOG_NODE)
            << "Restarting partitioned slot (" << reservation_->slot() << ").";
        complete(error::channel_stopped);
        return false;
    }

    // Request more blocks if our reservation has been expanded.
    send_get_blocks(complete, false);
    return true;
}

// This is fired by the base timer and stop handler.
void protocol_block_sync::handle_event(const code& ec, event_handler complete)
{
    if (ec == error::channel_stopped)
    {
        complete(ec);
        return;
    }

    if (ec && ec != error::channel_timeout)
    {
        log::debug(LOG_NODE)
            << "Failure in block sync timer for slot (" << reservation_->slot()
            << ") " << ec.message();
        complete(ec);
        return;
    }

    // This results from other channels taking this channel's hashes in
    // combination with this channel's peer not responding to the last request.
    // Causing a successful stop here prevents channel startup just to stop.
    if (reservation_->stopped())
    {
        log::debug(LOG_NODE)
            << "Stopping complete slot (" << reservation_->slot() << ").";
        complete(error::success);
        return;
    }

    if (reservation_->expired())
    {
        log::debug(LOG_NODE)
            << "Restarting slow slot (" << reservation_->slot() << ")";
        complete(error::channel_timeout);
        return;
    }
}

void protocol_block_sync::blocks_complete(const code& ec,
    event_handler handler)
{
    // We are no longer receiving blocks, so exclude from average.
    reservation_->reset();

    // This is the end of the peer sync sequence.
    // If there is no error the hash list must be empty and remain so.
    handler(ec);

    // The session does not need to handle the stop.
    stop(error::channel_stopped);
}

} // namespace node
} // namespace libbitcoin
