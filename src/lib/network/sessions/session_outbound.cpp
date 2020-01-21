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
#include <metaverse/network/sessions/session_outbound.hpp>

#include <cstddef>
#include <functional>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/p2p.hpp>
#include <metaverse/network/protocols/protocol_address.hpp>
#include <metaverse/network/protocols/protocol_ping.hpp>
#include <metaverse/bitcoin/utility/deadline.hpp>

namespace libbitcoin {
namespace network {

#define CLASS session_outbound

using namespace std::placeholders;

session_outbound::session_outbound(p2p& network)
  : session_batch(network, true),
    network__(network),
    CONSTRUCT_TRACK(session_outbound)
{
    outbound_counter = 0;
    in_reseeding = false;
}

// Start sequence.
// ----------------------------------------------------------------------------

void session_outbound::start(result_handler handler)
{
    if (settings_.outbound_connections == 0)
    {
        log::info(LOG_NETWORK)
            << "Not configured for generating outbound connections.";
        handler(error::success);
        return;
    }

    session::start(CONCURRENT2(handle_started, _1, handler));
}

void session_outbound::handle_started(const code& ec, result_handler handler)
{
    if (ec)
    {
        handler(ec);
        return;
    }

    outbound_counter = 0;
    in_reseeding = false;

    const auto connect = create_connector();
    for (size_t peer = 0; peer < settings_.outbound_connections; ++peer)
    {
        log::debug(LOG_NETWORK) << "new connection";
        new_connection(connect);
    }

    // This is the end of the start sequence.
    handler(error::success);
}

// Connnect cycle.
// ----------------------------------------------------------------------------

void session_outbound::new_connection(connector::ptr connect)
{
    if (stopped())
    {
        log::trace(LOG_NETWORK)
            << "Suspended outbound connection.";
        return;
    }
    static int i{0};
    int b = i++;
    this->connect(connect, BIND3(handle_connect, _1, _2, connect));
}

void session_outbound::delay_new_connect(connector::ptr connect)
{
    auto timer = std::make_shared<deadline>(pool_, asio::seconds(2));
    auto self = shared_from_this();
    timer->start([this, connect, timer, self](const code& ec){
        if (stopped())
        {
            return;
        }
        auto pThis = shared_from_this();
        auto action = [this, pThis, connect](){
            new_connection(connect);
        };
        pool_.service().post(action);
    });
}

void session_outbound::delay_reseeding()
{
    if (!network__.network_settings().enable_re_seeding) {
        return;
    }

    if (in_reseeding) {
        return;
    }
    in_reseeding = true;
    log::debug(LOG_NETWORK) << "outbound channel counter decreased, the re-seeding will be triggered 60s later!";
    auto timer = std::make_shared<deadline>(pool_, asio::seconds(60));
    auto self = shared_from_this();
    timer->start([this, timer, self](const code& ec){
        if (stopped())
        {
            return;
        }
        auto pThis = shared_from_this();
        auto action = [this, pThis](){
            const int counter = outbound_counter;
            if (counter > 1) {
                log::debug(LOG_NETWORK) << "outbound channel counter recovered to [" << counter << "], re-seeding is canceled!";
                in_reseeding = false;
                return;
            }
            log::debug(LOG_NETWORK) << "start re-seeding!";
            network__.restart_seeding();
            in_reseeding = false;
        };
        pool_.service().post(action);
    });
}

void session_outbound::handle_connect(const code& ec, channel::ptr channel,
    connector::ptr connect)
{
    if (ec)
    {
        log::trace(LOG_NETWORK)
            << "Failure connecting outbound: " << ec.message();
        delay_new_connect(connect);
        return;
    }

    log::trace(LOG_NETWORK)
        << "Connected to outbound channel [" << channel->authority() << "]";
    ++outbound_counter;
    register_channel(channel,
        BIND3(handle_channel_start, _1, connect, channel),
        BIND3(handle_channel_stop, _1, connect, channel));
}

void session_outbound::handle_channel_start(const code& ec,
    connector::ptr connect, channel::ptr channel)
{
    // Treat a start failure just like a stop.
    if (ec)
    {
        log::trace(LOG_NETWORK)
            << "Outbound channel failed to start ["
            << channel->authority() << "] " << ec.message();
        channel->invoke_protocol_start_handler(error::channel_stopped);
        channel->stop(ec);
        return;
    }

    attach_protocols(channel);
};

void session_outbound::attach_protocols(channel::ptr channel)
{
    attach<protocol_ping>(channel)->do_subscribe()->start();
    attach<protocol_address>(channel)->do_subscribe()->start();
}

void session_outbound::handle_channel_stop(const code& ec,
    connector::ptr connect, channel::ptr channel)
{
    channel->invoke_protocol_start_handler(error::channel_stopped);
    log::debug(LOG_NETWORK) << "channel stopped," << ec.message();

    const int counter = --outbound_counter;

    if(! stopped() && ec.value() != error::service_stopped)
    {
        delay_new_connect(connect);
        //restart the seeding procedure with in 1 minutes when outbound session count reduce to 1.
        if (counter <= 1) {
            delay_reseeding();
        }
    }
}



} // namespace network
} // namespace libbitcoin
