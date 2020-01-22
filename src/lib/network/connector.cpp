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
#include <boost/format.hpp>
#include <regex>

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

static std::string to_ipv4_hostname(const asio::address& ip_address)
{
    static const std::regex regular("^::ffff:([0-9\\.]+)$");

    const auto address = ip_address.to_string();
    std::sregex_iterator it(address.begin(), address.end(), regular), end;
    if (it == end)
        return "";

    const auto& match = *it;
    return match[1];
}

static std::string to_ipv6(const std::string& ipv4_address)
{
    return std::string("::ffff:") + ipv4_address;
}

static asio::ipv6 to_ipv6(const asio::ipv4& ipv4_address)
{
    // Create an IPv6 mapped IPv4 address via serialization.
    const auto ipv6 = to_ipv6(ipv4_address.to_string());
    return asio::ipv6::from_string(ipv6);
}

static asio::ipv6 to_ipv6(const asio::address& ip_address)
{
    if (ip_address.is_v6())
        return ip_address.to_v6();

    BITCOIN_ASSERT_MSG(ip_address.is_v4(),
        "The address must be either IPv4 or IPv6.");

    return to_ipv6(ip_address.to_v4());
}

static std::string to_ipv6_hostname(const asio::address& ip_address)
{
    // IPv6 URLs use a bracketed IPv6 address, see rfc2732.
    const auto hostname = boost::format("%1%") % to_ipv6(ip_address);
    return hostname.str();
}

static asio::ipv6 to_boost_address(const message::ip_address& in)
{
    asio::ipv6::bytes_type bytes;
    BITCOIN_ASSERT(bytes.size() == in.size());
    std::copy(in.begin(), in.end(), bytes.begin());
    const asio::ipv6 out(bytes);
    return out;
}

// public:
void connector::connect(const authority& authority, connect_handler handler
        , resolve_handler h)
{
//    connect(authority.to_hostname(), authority.port(), handler, h);
    auto ip = to_boost_address(authority.ip());
    auto ipv4_hostname = to_ipv4_hostname(ip);
    connect(ipv4_hostname.empty() ? to_ipv6_hostname(ip) : ipv4_hostname, authority.port(), handler, h);
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

    locked->get().async_connect(endpoint,
        std::bind(&connector::handle_connect,
            shared_from_this(), _1, socket, timer, handler));
    ///////////////////////////////////////////////////////////////////////////
}

// Timer sequence.
// ----------------------------------------------------------------------------

// private:
void connector::handle_timer(const code& ec, socket::ptr socket,
   connect_handler handler)
{
    // This is the end of the timer sequence.
    if (ec)
        handler(ec, nullptr);
    else
        handler(error::channel_timeout, nullptr);

    socket->close();
}

// Connect sequence.
// ----------------------------------------------------------------------------

// private:
void connector::handle_connect(const boost_code& ec,
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
