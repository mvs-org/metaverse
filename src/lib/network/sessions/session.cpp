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
#include <metaverse/network/sessions/session.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/acceptor.hpp>
#include <metaverse/network/channel.hpp>
#include <metaverse/network/connector.hpp>
#include <metaverse/network/p2p.hpp>
#include <metaverse/network/proxy.hpp>
#include <metaverse/network/protocols/protocol_address.hpp>
#include <metaverse/network/protocols/protocol_ping.hpp>
#include <metaverse/network/protocols/protocol_version.hpp>
#include <metaverse/network/settings.hpp>

namespace libbitcoin {
namespace network {

#define NAME "session"

// Base class binders.
#define BIND_0(method) \
    base_bind(&session::method)
#define BIND_1(method, p1) \
    base_bind(&session::method, p1)
#define BIND_2(method, p1, p2) \
    base_bind(&session::method, p1, p2)
#define BIND_3(method, p1, p2, p3) \
    base_bind(&session::method, p1, p2, p3)
#define BIND_4(method, p1, p2, p3, p4) \
    base_bind(&session::method, p1, p2, p3, p4)

using namespace std::placeholders;

session::session(p2p& network, bool outgoing, bool persistent)
  : stopped_(true),
    incoming_(!outgoing),
    notify_(persistent),
    network_(network),
    settings_(network.network_settings()),
    pool_(network.thread_pool()),
    dispatch_(pool_, NAME)
{
}

session::~session()
{
    BITCOIN_ASSERT_MSG(stopped(), "The session was not stopped.");
}

// Properties.
// ----------------------------------------------------------------------------

// protected:
void session::address_count(count_handler handler)
{
    network_.address_count(handler);
}

// protected:
void session::fetch_address(host_handler handler)
{
    network_.fetch_address(network_.authority_list(), handler);
}

void session::fetch_seed_address(host_handler handler)
{
    network_.fetch_seed_address(network_.authority_list(), handler);
}

// protected:
void session::connection_count(count_handler handler)
{
    network_.connected_count(handler);
}

// protected:
bool session::blacklisted(const authority& authority) const
{
    if (authority == settings_.self) {
        return true;
    }
    const auto& blocked = settings_.blacklists;
    // black through IP, does not care port.
    const auto it = std::find_if(blocked.begin(), blocked.end(),
        [&authority](const config::authority& elem){
            return (authority.ip() == elem.ip());
        });
    auto result = it != blocked.end();
    return result || channel::blacklisted(authority) || channel::manualbanned(authority);
}

void session::remove(const message::network_address& address, result_handler handler)
{
    network_.remove(address, handler);
}

void session::store(const message::network_address& address)
{
    network_.store(address, [](const code&){});
}

// Socket creators.
// ----------------------------------------------------------------------------
// Must not change context in the stop handlers (must use bind).

// protected:
acceptor::ptr session::create_acceptor()
{
    const auto accept = std::make_shared<acceptor>(pool_, settings_);
    subscribe_stop(BIND_2(do_stop_acceptor, _1, accept));
    return accept;
}

void session::do_stop_acceptor(const code&, acceptor::ptr accept)
{
    accept->stop();
}

// protected:
connector::ptr session::create_connector()
{
    const auto connect = std::make_shared<connector>(pool_, settings_);
    subscribe_stop(BIND_2(do_stop_connector, _1, connect));
    return connect;
}

void session::do_stop_connector(const code&, connector::ptr connect)
{
    connect->stop();
}

// Start sequence.
// ----------------------------------------------------------------------------
// Must not change context before subscribing.

void session::start(result_handler handler)
{
    if (!stopped())
    {
        handler(error::operation_failed);
        return;
    }

    stopped_ = false;
    subscribe_stop(BIND_1(do_stop_session, _1));

    // This is the end of the start sequence.
    handler(error::success);
}

void session::do_stop_session(const code&)
{
    // This signals the session to stop creating connections, but does not
    // close the session. Channels are stopped resulting in session lost scope.
    stopped_ = true;
}

bool session::stopped() const
{
    return stopped_;
}

bool session::stopped(const code& ec) const
{
    return stopped() || ec.value() == error::service_stopped;
}

// Subscribe Stop sequence.
// ----------------------------------------------------------------------------

void session::subscribe_stop(result_handler handler)
{
    network_.subscribe_stop(handler);
}

// Registration sequence.
// ----------------------------------------------------------------------------
// Must not change context in start or stop sequences (use bind).

// protected:
void session::register_channel(channel::ptr channel,
    result_handler handle_started, result_handler handle_stopped)
{
    result_handler stop_handler =
        BIND_3(do_remove, _1, channel, handle_stopped);

    result_handler start_handler =
        BIND_4(handle_start, _1, channel, handle_started, stop_handler);

    if (stopped())
    {
        start_handler(error::service_stopped);
        return;
    }

    if (incoming_)
    {
        handle_pend(error::success, channel, start_handler);
        return;
    }

    channel->set_notify(notify_);
    channel->set_nonce(nonzero_pseudo_random());

    result_handler unpend_handler =
        BIND_3(do_unpend, _1, channel, start_handler);

    pending_.store(channel,
        BIND_3(handle_pend, _1, channel, unpend_handler));
}

void session::handle_pend(const code& ec, channel::ptr channel,
    result_handler handle_started)
{
    if (ec)
    {
        handle_started(ec);
        return;
    }

    // The channel starts, invokes the handler, then starts the read cycle.
    channel->start(
        BIND_3(handle_channel_start, _1, channel, handle_started));
}

void session::handle_channel_start(const code& ec, channel::ptr channel,
    result_handler handle_started)
{
    if (ec)
    {
        log::info(LOG_NETWORK)
            << "Channel failed to start [" << channel->authority() << "] "
            << ec.message();
        handle_started(ec);
        return;
    }

    result_handler handshake_handler =
        BIND_3(handle_handshake, _1, channel, handle_started);

    attach_handshake_protocols(channel, handshake_handler);
}

// Sessions that desire to customize the version message must override this.
void session::attach_handshake_protocols(channel::ptr channel,
    result_handler handle_started)
{
    attach<protocol_version>(channel)->start(handle_started);
}

void session::handle_handshake(const code& ec, channel::ptr channel,
    result_handler handle_started)
{
    if (ec)
    {
        log::trace(LOG_NETWORK)
            << "Failure in handshake with [" << channel->authority()
            << "] " << ec.message();
        handle_started(ec);
        return;
    }

    truth_handler handler =
        BIND_3(handle_is_pending, _1, channel, handle_started);

    // The loopback test is for incoming channels only.
    if (incoming_)
        pending_.exists(channel->version().nonce, handler);
    else
        handler(false);
}

void session::handle_is_pending(bool pending, channel::ptr channel,
    result_handler handle_started)
{
    if (pending)
    {
        log::debug(LOG_NETWORK)
            << "Rejected connection from [" << channel->authority()
            << "] as loopback.";
        handle_started(error::accept_failed);
        return;
    }

    const auto& version = channel->version();

    if (version.value < version.minimum)
    {
        log::debug(LOG_NETWORK)
            << "Peer version (" << version.value << ") below minimum ("
            << version.minimum << ") [" << channel->authority() << "]";
        handle_started(error::accept_failed);
        return;
    }

    // This will fail if the IP address or nonce is already connected.
    network_.store(channel, handle_started);
}

void session::handle_start(const code& ec, channel::ptr channel,
    result_handler handle_started, result_handler handle_stopped)
{
    if (ec)
    {
        channel->stop(ec);
        handle_stopped(ec);
    }
    else
    {
        channel->subscribe_stop(handle_stopped);
    }

    // This is the end of the registration sequence.
    handle_started(ec);
}

void session::do_unpend(const code& ec, channel::ptr channel,
    result_handler handle_started)
{
    pending_.remove(channel, BIND_1(handle_unpend, _1));
    handle_started(ec);
}

void session::do_remove(const code& ec, channel::ptr channel,
    result_handler handle_stopped)
{
    network_.remove(channel, BIND_1(handle_remove, _1));
    handle_stopped(ec);
}

void session::handle_unpend(const code& ec)
{
    if (ec)
        log::trace(LOG_NETWORK)
            << "Failed to unpend a channel: " << ec.message();
}

void session::handle_remove(const code& ec)
{
    if (ec)
        log::trace(LOG_NETWORK)
            << "Failed to remove a channel: " << ec.message();
}

} // namespace network
} // namespace libbitcoin
