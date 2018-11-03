/**
 * Copyright (c) 2011-2016 libbitcoin developers (see AUTHORS)
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
#include <metaverse/network/sessions/session_batch.hpp>

#include <atomic>
#include <cstdint>
#include <memory>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/connector.hpp>
#include <metaverse/network/p2p.hpp>

namespace libbitcoin {
namespace network {

#define CLASS session_batch
#define NAME "session_batch"

using namespace bc::config;
using namespace std::placeholders;

session_batch::session_batch(p2p& network, bool persistent)
  : session(network, true, persistent),
    batch_size_(std::max(settings_.connect_batch_size, 1u))
{
}

void session_batch::converge(const code& ec, channel::ptr channel,
     atomic_counter_ptr counter, upgrade_mutex_ptr mutex,
     channel_handler handler)
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex->lock_upgrade();

    const auto initial_count = counter->load();
    BITCOIN_ASSERT(initial_count <= batch_size_);

    // Already completed, don't call handler.
    if (initial_count == batch_size_)
    {
        mutex->unlock_upgrade();
        //-----------------------------------------------------------------
        if (!ec)
            channel->stop(error::channel_stopped);

        return;
    }

    const auto count = !ec ? batch_size_ : initial_count + 1;
    const auto cleared = count == batch_size_;

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    mutex->unlock_upgrade_and_lock();
    counter->store(count);
    mutex->unlock();
    ///////////////////////////////////////////////////////////////////////

    if (cleared)
    {
        // If the last connection attempt is an error, normalize the code.
        auto result = ec ? error::operation_failed : error::success;
        handler(result, channel);
    }
}

// Connect sequence.
// ----------------------------------------------------------------------------

// protected:
void session_batch::connect(connector::ptr connect, channel_handler handler)
{
    // synchronizer state.
    const auto mutex = std::make_shared<upgrade_mutex>();
    const auto counter = std::make_shared<atomic_counter>(0);
    const auto singular = BIND5(converge, _1, _2, counter, mutex, handler);

    for (uint32_t host = 0; host < batch_size_; ++host)
        new_connect(connect, counter, singular);
}

void session_batch::new_connect(connector::ptr connect,
    atomic_counter_ptr counter, channel_handler handler)
{
    if (stopped())
    {
        log::debug(LOG_NETWORK)
            << "Suspended batch connection.";
        return;
    }

    if (counter->load() == batch_size_)
        return;
    fetch_address(BIND5(start_connect, _1, _2, connect, counter, handler));
}

void session_batch::start_connect(const code& ec, const authority& host,
    connector::ptr connect, atomic_counter_ptr counter, channel_handler handler)
{
    if (counter->load() == batch_size_ || ec.value() == error::service_stopped)
        return;

    // This termination prevents a tight loop in the empty address pool case.
    if (ec)
    {
        log::warning(LOG_NETWORK)
            << "Failure fetching new address: " << ec.message();
        handler(ec, nullptr);
        return;
    }

    // This creates a tight loop in the case of a small address pool.
    if (blacklisted(host))
    {
        handler(error::address_blocked, nullptr);
        return;
    }

    log::trace(LOG_NETWORK)
        << "Connecting to [" << host << "]";

    // CONNECT
    connect->connect(host, BIND7(handle_connect, _1, _2, host, connect,
        counter, 3, handler));
}

void session_batch::handle_connect(const code& ec, channel::ptr channel,
    const authority& host, connector::ptr connect, atomic_counter_ptr counter, std::size_t count,
    channel_handler handler)
{
    if (counter->load() == batch_size_)
        return;

    if (ec)
    {
        log::trace(LOG_NETWORK)
            << "Failure connecting to [" << host << "] " << count << ","
            << ec.message();
        if (ec.value() == error::channel_timeout) // if connect is not aviliable, change it into inactive state
            remove(host.to_network_address(), [](const code&){});
        handler(ec, channel);
        return;
    }

    store(host.to_network_address());

    log::trace(LOG_NETWORK)
        << "Connected to [" << channel->authority() << "]";

    // This is the end of the connect sequence.
    handler(error::success, channel);
}

} // namespace network
} // namespace libbitcoin
