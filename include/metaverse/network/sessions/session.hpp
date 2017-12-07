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
#ifndef MVS_NETWORK_SESSION_HPP
#define MVS_NETWORK_SESSION_HPP

#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/acceptor.hpp>
#include <metaverse/network/channel.hpp>
#include <metaverse/network/connections.hpp>
#include <metaverse/network/connector.hpp>
#include <metaverse/network/define.hpp>
#include <metaverse/network/pending_channels.hpp>
#include <metaverse/network/proxy.hpp>
#include <metaverse/network/settings.hpp>
#include <metaverse/bitcoin/message/network_address.hpp>

namespace libbitcoin {
namespace network {

// Base session type.

#define BASE_ARGS(handler, args) \
    std::forward<Handler>(handler), \
    shared_from_this(), \
    std::forward<Args>(args)...
#define BOUND_BASE(handler, args) \
    std::bind(BASE_ARGS(handler, args))

#define BASE_ARGS_TYPE(handler, args) \
    std::forward<Handler>(handler), \
    std::shared_ptr<session>(), \
    std::forward<Args>(args)...
#define BOUND_BASE_TYPE(handler, args) \
    std::bind(BASE_ARGS_TYPE(handler, args))

// Derived session types.

#define SESSION_ARGS(handler, args) \
    std::forward<Handler>(handler), \
    shared_from_base<Session>(), \
    std::forward<Args>(args)...
#define BOUND_SESSION(handler, args) \
    std::bind(SESSION_ARGS(handler, args))

#define SESSION_ARGS_TYPE(handler, args) \
    std::forward<Handler>(handler), \
    std::shared_ptr<Session>(), \
    std::forward<Args>(args)...
#define BOUND_SESSION_TYPE(handler, args) \
    std::bind(SESSION_ARGS_TYPE(handler, args))

class p2p;

/// Base class for maintaining the lifetime of a channel set, thread safe.
class BCT_API session
  : public enable_shared_from_base<session>
{
public:
    typedef std::shared_ptr<session> ptr;
    typedef config::authority authority;
    typedef std::function<void(bool)> truth_handler;
    typedef std::function<void(size_t)> count_handler;
    typedef std::function<void(const code&)> result_handler;
    typedef std::function<void(const code&, channel::ptr)> channel_handler;
    typedef std::function<void(const code&, acceptor::ptr)> accept_handler;
    typedef std::function<void(const code&, const authority&)> host_handler;

    /// Start the session, invokes handler once stop is registered.
    virtual void start(result_handler handler);

    /// Subscribe to receive session stop notification.
    virtual void subscribe_stop(result_handler handler);

protected:

    /// Construct an instance.
    session(p2p& network, bool outgoing, bool persistent);

    /// Validate session stopped.
    ~session();

    /// This class is not copyable.
    session(const session&) = delete;
    void operator=(const session&) = delete;

    /// Attach a protocol to a channel, caller must start the channel.
    template <class Protocol, typename... Args>
    typename Protocol::ptr attach(channel::ptr channel, Args&&... args)
    {
        return std::make_shared<Protocol>(network_, channel,
            std::forward<Args>(args)...);
    }

    /// Bind a method in the derived class.
    template <class Session, typename Handler, typename... Args>
    auto bind(Handler&& handler, Args&&... args) ->
        decltype(BOUND_SESSION_TYPE(handler, args))
    {
        return BOUND_SESSION(handler, args);
    }

    /////// Dispatch a concurrent method in the derived class.
    ////template <class Session, typename Handler, typename... Args>
    ////void concurrent(Handler&& handler, Args&&... args)
    ////{
    ////    return dispatch_.concurrent(SESSION_ARGS(handler, args));
    ////}

    /// Bind a concurrent delegate to a method in the derived class.
    template <class Session, typename Handler, typename... Args>
    auto concurrent_delegate(Handler&& handler, Args&&... args) ->
        delegates::concurrent<decltype(BOUND_SESSION_TYPE(handler, args))>
    {
        return dispatch_.concurrent_delegate(SESSION_ARGS(handler, args));
    }

    /// Properties.
    virtual void address_count(count_handler handler);
    virtual void fetch_address(host_handler handler);
    virtual void connection_count(count_handler handler);
    virtual bool blacklisted(const authority& authority) const;
    virtual bool stopped() const;

    void remove(const message::network_address& address, result_handler handler);

    void store(const message::network_address& address);

    /// Socket creators.
    virtual acceptor::ptr create_acceptor();
    virtual connector::ptr create_connector();

    /// Override to attach specialized handshake protocols upon session start.
    virtual void attach_handshake_protocols(channel::ptr channel,
        result_handler handle_started);

    /// Register a new channel with the session and bind its handlers.
    virtual void register_channel(channel::ptr channel,
        result_handler handle_started, result_handler handle_stopped);

    // TODO: create session_timer base class.
    threadpool& pool_;

    const settings& settings_;

private:

    /// Bind a method in the base class.
    template <typename Handler, typename... Args>
    auto base_bind(Handler&& handler, Args&&... args) ->
        decltype(BOUND_BASE_TYPE(handler, args))
    {
        return BOUND_BASE(handler, args);
    }

    // Socket creators.
    void do_stop_acceptor(const code& ec, acceptor::ptr connect);
    void do_stop_connector(const code& ec, connector::ptr connect);

    // Start sequence.
    void do_stop_session(const code&);

    // Connect sequence
    void new_connect(connector::ptr connect, channel_handler handler);
    void start_connect(const code& ec, const authority& host,
        connector::ptr connect, channel_handler handler);
    void handle_connect(const code& ec, channel::ptr channel,
        const authority& host, connector::ptr connect,
        channel_handler handler);

    // Registration sequence.
    void handle_channel_start(const code& ec, channel::ptr channel,
        result_handler handle_started);
    void handle_pend(const code& ec, channel::ptr channel,
        result_handler handle_started);
    void handle_handshake(const code& ec, channel::ptr channel,
        result_handler handle_started);
    void handle_is_pending(bool pending, channel::ptr channel,
        result_handler handle_started);
    void handle_start(const code& ec, channel::ptr channel,
        result_handler handle_started, result_handler handle_stopped);
    void do_unpend(const code& ec, channel::ptr channel,
        result_handler handle_started);
    void do_remove(const code& ec, channel::ptr channel,
        result_handler handle_stopped);
    void handle_unpend(const code& ec);
    void handle_remove(const code& ec);

    std::atomic<bool> stopped_;
    const bool incoming_;
    const bool notify_;

    // These are thread safe.
    p2p& network_;
    dispatcher dispatch_;
    pending_channels pending_;
};

// Base session type.

#undef BASE_ARGS
#undef BOUND_BASE
#undef BASE_ARGS_TYPE
#undef BOUND_BASE_TYPE

// Derived session types.

#undef SESSION_ARGS
#undef BOUND_SESSION
#undef SESSION_ARGS_TYPE
#undef BOUND_SESSION_TYPE

#define BIND1(method, p1) \
    bind<CLASS>(&CLASS::method, p1)
#define BIND2(method, p1, p2) \
    bind<CLASS>(&CLASS::method, p1, p2)
#define BIND3(method, p1, p2, p3) \
    bind<CLASS>(&CLASS::method, p1, p2, p3)
#define BIND4(method, p1, p2, p3, p4) \
    bind<CLASS>(&CLASS::method, p1, p2, p3, p4)
#define BIND5(method, p1, p2, p3, p4, p5) \
    bind<CLASS>(&CLASS::method, p1, p2, p3, p4, p5)
#define BIND6(method, p1, p2, p3, p4, p5, p6) \
    bind<CLASS>(&CLASS::method, p1, p2, p3, p4, p5, p6)
#define BIND7(method, p1, p2, p3, p4, p5, p6, p7) \
    bind<CLASS>(&CLASS::method, p1, p2, p3, p4, p5, p6, p7)

#define CONCURRENT2(method, p1, p2) \
    concurrent_delegate<CLASS>(&CLASS::method, p1, p2)


} // namespace network
} // namespace libbitcoin

#endif
