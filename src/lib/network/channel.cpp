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
#include <metaverse/network/channel.hpp>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <utility>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/proxy.hpp>
#include <metaverse/network/settings.hpp>
#include <metaverse/network/socket.hpp>

namespace libbitcoin {
namespace network {

using namespace bc::message;
using namespace std::placeholders;

// Factory for deadline timer pointer construction.
static deadline::ptr alarm(threadpool& pool, const asio::duration& duration)
{
    return std::make_shared<deadline>(pool, pseudo_randomize(duration));
}

// TODO: configure settings.protocol_maximum and settings.protocol_minimum.
// Limit to version::limit::maximum and version::limit::minimum respectively
// and if protocol_maximum is then below protocol_minimum return a failure. 
// On handshake send peer version.maxiumum and on receipt of protocol_peer
// if it is below protocol_minimum drop the channel, otherwise set
// protocol_version to the lesser of protocol_maximum and protocol_peer.
channel::channel(threadpool& pool, socket::ptr socket,
    const settings& settings)
  : proxy(pool, socket, settings.identifier, settings.protocol),
    notify_(false),
    nonce_(0),
    expiration_(alarm(pool, settings.channel_expiration())),
    inactivity_(alarm(pool, settings.channel_inactivity())),
    CONSTRUCT_TRACK(channel)
{
}

// Talk sequence.
// ----------------------------------------------------------------------------

// public:
void channel::start(result_handler handler)
{
    proxy::start(
        std::bind(&channel::do_start,
            shared_from_base<channel>(), _1, handler));
}

// Don't start the timers until the socket is enabled.
void channel::do_start(const code& ec, result_handler handler)
{
    start_expiration();
    start_inactivity();
    handler(error::success);
}

// Properties (version write is thread unsafe, isolate from read).
// ----------------------------------------------------------------------------

bool channel::notify() const
{
    return notify_;
}

void channel::set_notify(bool value)
{
    notify_ = value;
}

uint64_t channel::nonce() const
{
    return nonce_;
}

void channel::set_nonce(uint64_t value)
{
    nonce_ = value;
}

void channel::set_protocol_start_handler(std::function<void()> handler)
{
    protocol_start_handler_ = handler;
}

void channel::invoke_protocol_start_handler(const code& ec)
{
    std::function<void()> func;
    {
        unique_lock lock{mutex_};
        if (!protocol_start_handler_)
            return;
        if (ec) {
    	    protocol_start_handler_ = nullptr;
    	    return;
        }

        func = std::move(protocol_start_handler_);
        protocol_start_handler_ = nullptr;
    }
    func();
}

// Proxy pure virtual protected and ordered handlers.
// ----------------------------------------------------------------------------

// It is possible that this may be called multiple times.
void channel::handle_stopping()
{
	invoke_protocol_start_handler(error::channel_stopped);
    expiration_->stop();
    inactivity_->stop();
}

void channel::handle_activity()
{
    start_inactivity();
}

// Timers (these are inherent races, requiring stranding by stop only).
// ----------------------------------------------------------------------------

void channel::start_expiration()
{
    if (stopped())
        return;

    expiration_->start(
        std::bind(&channel::handle_expiration,
            shared_from_base<channel>(), _1));
}

void channel::handle_expiration(const code& ec)
{
    if (stopped())
        return;

    log::debug(LOG_NETWORK)
        << "Channel lifetime expired [" << authority() << "]";

    stop(error::channel_timeout);
}

void channel::start_inactivity()
{
    if (stopped())
        return;

    inactivity_->start(
        std::bind(&channel::handle_inactivity,
            shared_from_base<channel>(), _1));
}

void channel::handle_inactivity(const code& ec)
{
    if (stopped())
        return;

    log::debug(LOG_NETWORK)
        << "Channel inactivity timeout [" << authority() << "]";

    stop(error::channel_timeout);
}

} // namespace network
} // namespace libbitcoin
