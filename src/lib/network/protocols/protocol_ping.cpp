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
#include <metaverse/network/protocols/protocol_ping.hpp>

#include <cstdint>
#include <functional>
#include <string>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/channel.hpp>
#include <metaverse/network/p2p.hpp>
#include <metaverse/network/protocols/protocol_timer.hpp>

namespace libbitcoin {
namespace network {

#define NAME "ping"
#define CLASS protocol_ping

using namespace bc::message;
using namespace std::placeholders;

protocol_ping::protocol_ping(p2p& network, channel::ptr channel)
  : protocol_timer(network, channel, true, NAME),
    settings_(network.network_settings()),
    CONSTRUCT_TRACK(protocol_ping)
{
}

void protocol_ping::start(result_handler handler)
{
    protocol_timer::start(settings_.channel_heartbeat(), BIND1(send_ping, _1));
    {
    	unique_lock lock{mutex_};
    	result_handler_ = handler;
    }
    SUBSCRIBE2(ping, handle_receive_ping, _1, _2);

    // Send initial ping message by simulating first heartbeat.
    set_event(error::success);
}

void protocol_ping::handle_or_not(uint64_t nonce)
{
	shared_lock lock{mutex_};
	if (result_handler_)
	{
		log::trace(LOG_NETWORK) << "handle or not";
		SEND2(ping{ nonce }, handle_send, _1, pong::command);
	}
}

// This is fired by the callback (i.e. base timer and stop handler).
void protocol_ping::send_ping(const code& ec)
{
    if (stopped())
    {
    	log::trace(LOG_NETWORK) << "protocol_ping::send ping stopped" ;
    	test_call_handler(error::channel_stopped);
        return;
    }

    if (ec && ec != error::channel_timeout)
    {
        log::trace(LOG_NETWORK)
            << "Failure in ping timer for [" << authority() << "] "
            << ec.message();
        test_call_handler(ec);
        stop(ec);
        return;
    }

    const auto nonce = pseudo_random();

    SUBSCRIBE3(pong, handle_receive_pong, _1, _2, nonce);
    SEND2(ping{ nonce }, handle_send, _1, pong::command);

    shared_lock lock{mutex_};
    if(result_handler_)
    {
    	auto line = std::make_shared<deadline>(pool(), asio::seconds{1});
    	auto pThis = shared_from_this();
    	line->start([pThis, line, nonce](const code& ec){
			static_cast<protocol_ping*>(pThis.get())->handle_or_not(nonce);
		});
    }
}

void protocol_ping::test_call_handler(const code& ec)
{
	upgrade_lock upgrade{mutex_};
	if(result_handler_)
	{
		log::trace(LOG_NETWORK) << "test call handler";
		unique_lock lock(std::move(upgrade));
		auto handler = std::move(result_handler_);
		auto action = [handler, ec](){
			handler(ec);
		};
		pool().service().post(action);
		result_handler_ = nullptr;
	}
}

bool protocol_ping::handle_receive_ping(const code& ec,
    message::ping::ptr message)
{
    if (stopped())
    {
    	log::trace(LOG_NETWORK) << "protocol_ping::handle_receive_ping stopped" ;
    	test_call_handler(error::channel_stopped);
        return false;
    }

    if (ec)
    {
        log::trace(LOG_NETWORK)
            << "Failure getting ping from [" << authority() << "] "
            << ec.message();
        test_call_handler(ec);
        stop(ec);
        return false;
    }
    SEND2(pong{ message->nonce }, handle_send, _1, pong::command);

    // RESUBSCRIBE
    return true;
}

bool protocol_ping::handle_receive_pong(const code& ec,
    message::pong::ptr message, uint64_t nonce)
{
    if (stopped())
    {
    	log::trace(LOG_NETWORK) << "protocol_ping::handle_receive_pong stopped" ;
    	test_call_handler(error::channel_stopped);
        return false;
    }

    test_call_handler(ec);

    if (ec)
    {
        log::trace(LOG_NETWORK)
            << "Failure getting pong from [" << authority() << "] "
            << ec.message();
        stop(ec);
        return false;
    }

    if (message->nonce != nonce)
    {
        log::warning(LOG_NETWORK)
            << "Invalid pong nonce from [" << authority() << "]";

        // This could result from message overlap due to a short period,
        // but we assume the response is not as expected and terminate.
        stop(error::bad_stream);
    }

    return false;
}

} // namespace network
} // namespace libbitcoin
