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
#include <bitcoin/node/protocols/protocol_header_sync.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <bitcoin/network.hpp>
#include <bitcoin/node/p2p_node.hpp>
#include <bitcoin/node/utility/header_queue.hpp>

namespace libbitcoin {
namespace node {

#define NAME "header_sync"
#define CLASS protocol_header_sync

using namespace bc::config;
using namespace bc::message;
using namespace bc::network;
using namespace std::placeholders;

// The protocol maximum size for get data header requests.
static constexpr size_t max_header_response = 2000;

// The interval in which header download rate is measured and tested.
static const asio::seconds expiry_interval(5);

// This class requires protocol version 31800.
protocol_header_sync::protocol_header_sync(p2p& network,
    channel::ptr channel, header_queue& hashes, uint32_t minimum_rate,
    const checkpoint& last)
  : protocol_timer(network, channel, true, NAME),
    hashes_(hashes),
    current_second_(0),
    minimum_rate_(minimum_rate),
    start_size_(hashes.size()),
    last_(last),
    CONSTRUCT_TRACK(protocol_header_sync)
{
}

// Utilities
// ----------------------------------------------------------------------------

size_t protocol_header_sync::next_height() const
{
    return hashes_.last_height() + 1;
}

size_t protocol_header_sync::sync_rate() const
{
    // We can never roll back prior to start size since it's min final height.
    return (hashes_.size() - start_size_) / current_second_;
}

// Start sequence.
// ----------------------------------------------------------------------------

void protocol_header_sync::start(event_handler handler)
{
    auto complete = synchronize(BIND2(headers_complete, _1, handler), 1, NAME);
    protocol_timer::start(expiry_interval, BIND2(handle_event, _1, complete));

    SUBSCRIBE3(headers, handle_receive, _1, _2, complete);

    // This is the end of the start sequence.
    send_get_headers(complete);
}

// Header sync sequence.
// ----------------------------------------------------------------------------

void protocol_header_sync::send_get_headers(event_handler complete)
{
    if (stopped())
        return;

    const get_headers request
    {
        { hashes_.last_hash() },
        last_.hash()
    };

    SEND2(request, handle_send, _1, complete);
}

void protocol_header_sync::handle_send(const code& ec, event_handler complete)
{
    if (stopped())
        return;

    if (ec)
    {
        log::debug(LOG_NODE)
            << "Failure sending get headers to sync [" << authority() << "] "
            << ec.message();
        complete(ec);
    }
}

bool protocol_header_sync::handle_receive(const code& ec, headers_ptr message,
    event_handler complete)
{
    if (stopped())
        return false;

    if (ec)
    {
        log::debug(LOG_NODE)
            << "Failure receiving headers from sync ["
            << authority() << "] " << ec.message();
        complete(ec);
        return false;
    }

    // A merge failure includes automatic rollback to last trust point.
    if (!hashes_.enqueue(message))
    {
        log::warning(LOG_NODE)
            << "Failure merging headers from [" << authority() << "]";
        complete(error::previous_block_invalid);
        return false;
    }

    const auto next = next_height();

    log::info(LOG_NODE)
        << "Synced headers " << next - message->elements.size()
        << "-" << (next - 1) << " from [" << authority() << "]";

    // If we completed the last height the sync is complete/success.
    if (next > last_.height())
    {
        complete(error::success);
        return false;
    }

    // If we received fewer than 2000 the peer is exhausted, try another.
    if (message->elements.size() < max_header_response)
    {
        complete(error::operation_failed);
        return false;
    }

    // This peer has more headers.
    send_get_headers(complete);
    return true;
}

// This is fired by the base timer and stop handler.
void protocol_header_sync::handle_event(const code& ec, event_handler complete)
{
    if (ec == error::channel_stopped)
    {
        complete(ec);
        return;
    }

    if (ec && ec != error::channel_timeout)
    {
        log::warning(LOG_NODE)
            << "Failure in header sync timer for [" << authority() << "] "
            << ec.message();
        complete(ec);
        return;
    }

    // It was a timeout, so ten more seconds have passed.
    current_second_ += expiry_interval.count();

    // Drop the channel if it falls below the min sync rate averaged over all.
    if (sync_rate() < minimum_rate_)
    {
        log::debug(LOG_NODE)
            << "Header sync rate (" << sync_rate() << "/sec) from ["
            << authority() << "]";
        complete(error::channel_timeout);
        return;
    }
}

void protocol_header_sync::headers_complete(const code& ec,
    event_handler handler)
{
    // This is end of the header sync sequence.
    handler(ec);

    // The session does not need to handle the stop.
    stop(error::channel_stopped);
}

} // namespace node
} // namespace libbitcoin
