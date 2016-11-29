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
#include <bitcoin/network/sessions/session_outbound.hpp>

#include <cstddef>
#include <functional>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/network/p2p.hpp>
#include <bitcoin/network/protocols/protocol_address.hpp>
#include <bitcoin/network/protocols/protocol_ping.hpp>
#include <bitcoin/bitcoin/utility/deadline.hpp>

namespace libbitcoin {
namespace network {

#define CLASS session_outbound

using namespace std::placeholders;

session_outbound::session_outbound(p2p& network)
  : session_batch(network, true),
    CONSTRUCT_TRACK(session_outbound)
{
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

    const auto connect = create_connector();
    for (size_t peer = 0; peer < settings_.outbound_connections; ++peer)
        new_connection(connect);

    // This is the end of the start sequence.
    handler(error::success);
}

// Connnect cycle.
// ----------------------------------------------------------------------------

void session_outbound::new_connection(connector::ptr connect)
{
    if (stopped())
    {
        log::debug(LOG_NETWORK)
            << "Suspended outbound connection.";
        return;
    }

    this->connect(connect, BIND3(handle_connect, _1, _2, connect));
}

void session_outbound::delay_new_connect(connector::ptr connect)
{
	log::debug(LOG_NETWORK) << "delay new connect" ;
	auto timer = std::make_shared<deadline>(pool_, asio::seconds(2));
	auto self = shared_from_this();
	timer->start([this, connect, timer, self](const code& ec){
		if (ec)
		{
			log::debug(LOG_NETWORK) << "delay new connect, " << ec.message() ;
		}

		if (stopped())
		{
			log::debug(LOG_NETWORK) << "delay new connect, session stopped" ;
			return;
		}
		new_connection(connect);
	});
}

void session_outbound::handle_connect(const code& ec, channel::ptr channel,
    connector::ptr connect)
{
    if (ec)
    {
        log::debug(LOG_NETWORK)
            << "Failure connecting outbound: " << ec.message();
        if(ec.value() == error::not_satisfied)
		{
        	log::debug(LOG_NETWORK) << "session outbound handle connect, not satified";
			return;
		}
        delay_new_connect(connect);
        return;
    }

    log::info(LOG_NETWORK)
        << "Connected to outbound channel [" << channel->authority() << "]";

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
        log::debug(LOG_NETWORK)
            << "Outbound channel failed to start ["
            << channel->authority() << "] " << ec.message();
        return;
    }

    attach_protocols(channel);
};

void session_outbound::attach_protocols(channel::ptr channel)
{
    attach<protocol_ping>(channel)->start([](const code&){});
    attach<protocol_address>(channel)->start();
}

void session_outbound::handle_channel_stop(const code& ec,
    connector::ptr connect, channel::ptr channel)
{
    log::debug(LOG_NETWORK)
        << "Outbound channel stopped [" << channel->authority() << "] "
        << ec.message();

    if(not stopped() and ec.value() != error::service_stopped)
    {
    	delay_new_connect(connect);
    }
}

} // namespace network
} // namespace libbitcoin
