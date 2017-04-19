/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS)
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
#include <metaverse/network/sessions/session_manual.hpp>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/p2p.hpp>
#include <metaverse/network/protocols/protocol_address.hpp>
#include <metaverse/network/protocols/protocol_ping.hpp>

namespace libbitcoin {
namespace network {

#define CLASS session_manual

using namespace std::placeholders;

session_manual::session_manual(p2p& network)
  : session_batch(network, true),
    CONSTRUCT_TRACK(session_manual)
{
}

// Start sequence.
// ----------------------------------------------------------------------------
// Manual connections are always enabled.

void session_manual::start(result_handler handler)
{
    session::start(CONCURRENT2(handle_started, _1, handler));
}

void session_manual::handle_started(const code& ec, result_handler handler)
{
    if (ec)
    {
        handler(ec);
        return;
    }

    connector_.store(create_connector());

    // This is the end of the start sequence.
    handler(error::success);
}

// Connect sequence/cycle.
// ----------------------------------------------------------------------------

void session_manual::connect(const std::string& hostname, uint16_t port)
{
    const auto unhandled = [](code, channel::ptr) {};
    connect(hostname, port, unhandled);
}

void session_manual::connect(const std::string& hostname, uint16_t port,
    channel_handler handler)
{
    start_connect(hostname, port, handler, settings_.manual_attempt_limit);
}

// The first connect is a sequence, which then spawns a cycle.
void session_manual::start_connect(const std::string& hostname, uint16_t port,
    channel_handler handler, uint32_t retries)
{
    if (stopped())
    {
        log::debug(LOG_NETWORK)
            << "Suspended manual connection.";

        connector_.store(nullptr);
        handler(error::service_stopped, nullptr);
        return;
    }

    auto connector = connector_.load();
    BITCOIN_ASSERT_MSG(connector, "The manual session was not started.");

    // MANUAL CONNECT OUTBOUND
    connector->connect(hostname, port,
        BIND6(handle_connect, _1, _2, hostname, port, handler, retries));
}

void session_manual::handle_connect(const code& ec, channel::ptr channel,
    const std::string& hostname, uint16_t port, channel_handler handler,
    uint32_t retries)
{
    if (ec)
    {
        log::warning(LOG_NETWORK)
            << "Failure connecting [" << config::endpoint(hostname, port)
            << "] manually: " << ec.message();

        // Retry logic.
        if (settings_.manual_attempt_limit == 0)
            start_connect(hostname, port, handler, 0);
        else if (retries > 0)
            start_connect(hostname, port, handler, retries - 1);
        else
            handler(ec, nullptr);

        return;
    }

    log::info(LOG_NETWORK)
        << "Connected manual channel [" << config::endpoint(hostname, port)
        << "] as [" << channel->authority() << "]";

    register_channel(channel, 
        BIND5(handle_channel_start, _1, hostname, port, channel, handler),
        BIND3(handle_channel_stop, _1, hostname, port));
}

void session_manual::handle_channel_start(const code& ec,
    const std::string& hostname, uint16_t port, channel::ptr channel,
    channel_handler handler)
{
    // Treat a start failure just like a stop, but preserve the start handler.
    if (ec)
    {
        log::info(LOG_NETWORK)
            << "Manual channel failed to start [" << channel->authority()
            << "] " << ec.message();

        // Special case for already connected, do not keep trying.
        if (ec == (code)error::address_in_use)
        {
            handler(ec, channel);
            return;
        }

        connect(hostname, port, handler);
        return;
    }

    // This is the end of the connect sequence (the handler goes out of scope).
    handler(error::success, channel);

    // This is the beginning of the connect sequence.
    attach_protocols(channel);
};

void session_manual::attach_protocols(channel::ptr channel)
{
    attach<protocol_ping>(channel)->start([](const code&){});
    attach<protocol_address>(channel)->start();
}

// After a stop we don't use the caller's start handler, but keep connecting.
void session_manual::handle_channel_stop(const code& ec,
    const std::string& hostname, uint16_t port)
{
    log::debug(LOG_NETWORK)
        << "Manual channel stopped: " << ec.message();

    connect(hostname, port);
}

} // namespace network
} // namespace libbitcoin
