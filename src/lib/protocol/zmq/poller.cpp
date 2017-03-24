/**
 * Copyright (c) 2011-2016 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin-protocol.
 *
 * libbitcoin-protocol is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
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
#include <metaverse/lib/protocol/zmq/poller.hpp>

#include <cstdint>
#include <zmq.h>
#include <metaverse/lib/bitcoin.hpp>
#include <metaverse/lib/protocol/zmq/identifiers.hpp>
#include <metaverse/lib/protocol/zmq/socket.hpp>

namespace libbitcoin {
namespace protocol {
namespace zmq {

poller::poller()
  : expired_(false),
    terminated_(false)
{
}

// Parameter fd is non-zmq socket (unused when socket is set).
void poller::add(socket& socket)
{
    zmq_pollitem item;
    item.socket = socket.self();
    item.fd = 0;
    item.events = ZMQ_POLLIN;
    item.revents = 0;

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    unique_lock lock(mutex_);

    pollers_.push_back(item);
    ///////////////////////////////////////////////////////////////////////////
}

void poller::clear()
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    unique_lock lock(mutex_);

    return pollers_.clear();
    ///////////////////////////////////////////////////////////////////////////
}

// This must be called on the socket thread.
identifiers poller::wait()
{
    // This is the maximum safe value on all platforms, due to zeromq bug.
    static constexpr int32_t maximum_safe_wait_milliseconds = 1000;

    return wait(maximum_safe_wait_milliseconds);
}

// This must be called on the socket thread.
// BUGBUG: zeromq 4.2 has an overflow bug in timer parameterization.
// The timeout is typed as 'long' by zeromq. This is 32 bit on windows and
// actually less (potentially 1000 or 1 second) on other platforms.
// On non-windows platforms negative doesn't actually produce infinity.
identifiers poller::wait(int32_t timeout_milliseconds)
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    shared_lock lock(mutex_);

    const auto size = pollers_.size();
    BITCOIN_ASSERT(size <= max_int32);

    const auto item_count = static_cast<int32_t>(size);
    const auto items = reinterpret_cast<zmq_pollitem_t*>(pollers_.data());
    const auto signaled = zmq_poll(items, item_count, timeout_milliseconds);

    // Either one of the sockets was terminated or a signal intervened.
    if (signaled < 0)
    {
        terminated_ = true;
        return{};
    }

    // No events have been signaled and no failure, so ther timer expired.
    if (signaled == 0)
    {
        expired_ = true;
        return{};
    }

    identifiers result;
    for (const auto& poller: pollers_)
        if ((poller.revents & ZMQ_POLLIN) != 0)
            result.push(poller.socket);

    // At least one event was signaled, but this poll-in set may be empty.
    return result;
    ///////////////////////////////////////////////////////////////////////////
}

bool poller::expired() const
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    shared_lock lock(mutex_);

    return expired_;
    ///////////////////////////////////////////////////////////////////////////
}

bool poller::terminated() const
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    shared_lock lock(mutex_);

    return terminated_;
    ///////////////////////////////////////////////////////////////////////////
}

} // namespace zmq
} // namespace protocol
} // namespace libbitcoin
