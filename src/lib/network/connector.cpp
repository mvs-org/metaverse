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
#include <metaverse/network/connector.hpp>

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/channel.hpp>
#include <metaverse/network/proxy.hpp>
#include <metaverse/network/settings.hpp>
#include <metaverse/network/socket.hpp>

namespace libbitcoin {
namespace network {

#define NAME "connector"
    
using namespace bc::config;
using namespace std::placeholders;

// The resolver_, pending_, and stopped_ members are protected.

connector::connector(threadpool& pool, const settings& settings)
  : stopped_(false),
    pool_(pool),
    settings_(settings),
    dispatch_(pool, NAME),
    resolver_(std::make_shared<asio::resolver>(pool.service())),
    CONSTRUCT_TRACK(connector)
{
}

// Stop sequence.
// ----------------------------------------------------------------------------

// public:
void connector::stop()
{
    safe_stop();
    pending_.clear();
}

void connector::safe_stop()
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_upgrade();

    if (!stopped_)
    {
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        mutex_.unlock_upgrade_and_lock();

        // This will asynchronously invoke the handler of each pending resolve.
        resolver_->cancel();
        stopped_ = true;

        mutex_.unlock();
        //---------------------------------------------------------------------
        return;
    }

    mutex_.unlock_upgrade();
    ///////////////////////////////////////////////////////////////////////////
}

bool connector::stopped()
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    shared_lock lock(mutex_);

    return stopped_;
    ///////////////////////////////////////////////////////////////////////////
}

// Connect sequence.
// ----------------------------------------------------------------------------

// public:
void connector::connect(const endpoint& endpoint, connect_handler handler
		, resolve_handler h)
{
    connect(endpoint.host(), endpoint.port(), handler, h);
}

// public:
void connector::connect(const authority& authority, connect_handler handler
		, resolve_handler h)
{
    connect(authority.to_hostname(), authority.port(), handler, h);
}

// public:
void connector::connect(const std::string& hostname, uint16_t port,
    connect_handler handler, resolve_handler h)
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_upgrade();

    if (stopped_)
    {
        // We preserve the asynchronous contract of the async_resolve.
        // Dispatch ensures job does not execute in the current thread.
        dispatch_.concurrent(handler, error::service_stopped, nullptr);
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        return;
    }

    auto query = std::make_shared<asio::query>(hostname, std::to_string(port));

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    mutex_.unlock_upgrade_and_lock();

    // async_resolve will not invoke the handler within this function.
    resolver_->async_resolve(*query,
        std::bind(&connector::handle_resolve,
            shared_from_this(), _1, _2, handler, h));

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////
}

void connector::handle_resolve(const boost_code& ec, asio::iterator iterator,
    connect_handler handler, resolve_handler h)
{
	auto it = iterator;
	asio::iterator end;
	if (h)
	{
		while(it != end){
			h(*it);
			++it;
		}
	}
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_shared();

    if (stopped_)
    {
        dispatch_.concurrent(handler, error::service_stopped, nullptr);
        mutex_.unlock_shared();
        //---------------------------------------------------------------------
        return;
    }

    if (ec)
    {
        dispatch_.concurrent(handler, error::resolve_failed, nullptr);
        mutex_.unlock_shared();
        //---------------------------------------------------------------------
        return;
    }

    auto do_connecting = [this, &handler](asio::iterator resolver_iterator)
    {
        const auto timeout = settings_.connect_timeout();
        const auto timer = std::make_shared<deadline>(pool_, timeout);
        const auto socket = std::make_shared<network::socket>(pool_);
    
        // Retain a socket reference until connected, allowing connect cancelation.
        pending_.store(socket);
    
        // Manage the socket-timer race, terminating if either fails.
        const auto handle_connect = synchronize(handler, 1, NAME, false);
    
        // This is branch #1 of the connnect sequence.
        timer->start(
            std::bind(&connector::handle_timer,
                shared_from_this(), _1, socket, handle_connect));
    
        safe_connect(resolver_iterator, socket, timer, handle_connect);
    };
    
    // Get all hosts under one DNS record.
    for (asio::iterator end; iterator != end; ++iterator)
    {  
        do_connecting(iterator);
        break; // FIXME. chenhao, can not query all hosts, caused by loop fastly
    }

    mutex_.unlock_shared();
    ///////////////////////////////////////////////////////////////////////////
}

void connector::safe_connect(asio::iterator iterator, socket::ptr socket,
    deadline::ptr timer, connect_handler handler)
{
    // Critical Section (external)
    /////////////////////////////////////////////////////////////////////////// 
    const auto locked = socket->get_socket();

    // This is branch #2 of the connnect sequence.
    using namespace boost::asio;
	ip::tcp::endpoint endpoint = *iterator;
	bc::log::trace(NAME)<< "conecting addr:" << endpoint;

    async_connect(locked->get(), iterator,
        std::bind(&connector::handle_connect,
            shared_from_this(), _1, _2, socket, timer, handler));
    /////////////////////////////////////////////////////////////////////////// 
}

// Timer sequence.
// ----------------------------------------------------------------------------

// private:
void connector::handle_timer(const code& ec, socket::ptr socket,
   connect_handler handler)
{
    // This is the end of the timer sequence.
	bc::log::trace(NAME)<<"timer called here: "<<__func__;
    if (ec)
        handler(ec, nullptr);
    else
        handler(error::channel_timeout, nullptr);

    socket->close();
}

// Connect sequence.
// ----------------------------------------------------------------------------

// private:
void connector::handle_connect(const boost_code& ec, asio::iterator iter,
    socket::ptr socket, deadline::ptr timer, connect_handler handler)
{
    pending_.remove(socket);
    // This is the end of the connect sequence.
    if (ec)
        handler(error::boost_to_error_code(ec), nullptr);
    else
        handler(error::success, new_channel(socket));

    timer->stop();
}

std::shared_ptr<channel> connector::new_channel(socket::ptr socket)
{
    return std::make_shared<channel>(pool_, socket, settings_);
}

} // namespace network
} // namespace libbitcoin
