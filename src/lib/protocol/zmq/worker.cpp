/*
 * Copyright (c) 2011-2021 libbitcoin developers (see AUTHORS)
 *
 * This file is part of metaverse-protocol.
 *
 * metaverse-protocol is free software: you can redistribute it and/or
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
#include <metaverse/protocol/zmq/worker.hpp>

#include <zmq.h>
#include <functional>
#include <future>
#include <metaverse/bitcoin.hpp>
#include <metaverse/protocol/zmq/message.hpp>
#include <metaverse/protocol/zmq/socket.hpp>

namespace libbitcoin {
namespace protocol {
namespace zmq {

#define NAME "worker"

// Derive from this abstract worker to implement real worker.
worker::worker(threadpool& pool)
  : dispatch_(pool, NAME),
    stopped_(true)
{
}

worker::~worker()
{
    stop();
}

// Restartable after stop and not started on construct.
bool worker::start()
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    unique_lock lock(mutex_);

    if (stopped_)
    {
        stopped_ = false;

        // Create the replier thread and socket and start polling.
        dispatch_.concurrent(
            std::bind(&worker::work, this));

        // Wait on replier start.
        const auto result = started_.get_future().get();

        // Reset for restartability.
        started_ = std::promise<bool>();
        return result;
    }

    return false;
    ///////////////////////////////////////////////////////////////////////////
}

bool worker::stop()
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    unique_lock lock(mutex_);

    if (!stopped_)
    {
        stopped_ = true;

        // Wait on replier stop.
        const auto result = finished_.get_future().get();

        // Reset for restartability.
        finished_ = std::promise<bool>();
        return result;
    }

    return true;
    ///////////////////////////////////////////////////////////////////////////
}

// Utilities.
//-----------------------------------------------------------------------------

// Call from work to detect an explicit stop.
bool worker::stopped()
{
    return stopped_;
}

bool worker::stopped(const code& ec)
{
    return stopped() ||
        ec.value() == error::service_stopped ||
        ec.value() == error::channel_stopped;
}

// Call from work when started (connected/bound) or failed to do so.
bool worker::started(bool result)
{
    started_.set_value(result);

    if (!result)
        finished(true);

    return result;
}

// Call from work when finished working, do not call if started was called.
bool worker::finished(bool result)
{
    finished_.set_value(result);
    return result;
}

// Call from work to forward a message from one socket to another.
bool worker::forward(socket& from, socket& to)
{
    message packet;
    return !from.receive(packet) && !to.send(packet);
}

// Call from work to establish a proxy between two sockets.
void worker::relay(socket& left, socket& right)
{
    // Blocks until the context is terminated, always returns -1.
    zmq_proxy_steerable(left.self(), right.self(), nullptr, nullptr);

    // Equivalent implementation:
    ////zmq::poller poller;
    ////poller.add(left);
    ////poller.add(right);
    ////
    ////while (!poller.terminated())
    ////{
    ////    const auto signaled = poller.wait();
    ////
    ////    if (signaled.contains(left.id()))
    ////        forward(left, right);
    ////
    ////    if (signaled.contains(right.id()))
    ////        forward(right, left);
    ////}
}

} // namespace zmq
} // namespace protocol
} // namespace libbitcoin
