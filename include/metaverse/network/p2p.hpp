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
#ifndef MVS_NETWORK_P2P_HPP
#define MVS_NETWORK_P2P_HPP

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/channel.hpp>
#include <metaverse/network/connections.hpp>
#include <metaverse/network/define.hpp>
#include <metaverse/network/hosts.hpp>
#include <metaverse/network/message_subscriber.hpp>
#include <metaverse/network/sessions/session_inbound.hpp>
#include <metaverse/network/sessions/session_manual.hpp>
#include <metaverse/network/sessions/session_outbound.hpp>
#include <metaverse/network/sessions/session_seed.hpp>
#include <metaverse/network/settings.hpp>

namespace libbitcoin {
namespace network {

/// Top level public networking interface, partly thread safe.
class BCT_API p2p
  : public enable_shared_from_base<p2p>
{
public:
    typedef std::shared_ptr<p2p> ptr;
    typedef message::network_address address;
    typedef std::function<void()> stop_handler;
    typedef std::function<void(bool)> truth_handler;
    typedef std::function<void(size_t)> count_handler;
    typedef std::function<void(const code&)> result_handler;
    typedef std::function<void(const code&, const address&)> address_handler;
    typedef std::function<void(const code&, channel::ptr)> channel_handler;
    typedef std::function<bool(const code&, channel::ptr)> connect_handler;
    typedef subscriber<const code&> stop_subscriber;
    typedef resubscriber<const code&, channel::ptr> channel_subscriber;

    // Templates (send/receive).
    // ------------------------------------------------------------------------

    /// Send message to all connections.
    template <typename Message>
    void broadcast(Message&& message, channel_handler handle_channel,
        result_handler handle_complete)
    {
        connections_->broadcast(message, handle_channel, handle_complete);
    }

    /// Subscribe to all incoming messages of a type.
    template <class Message>
    void subscribe(message_handler<Message>&& handler)
    {
        connections_->subscribe(
            std::forward<message_handler<Message>>(handler));
    }

    // Constructors.
    // ------------------------------------------------------------------------

    /// Construct an instance.
    p2p(const settings& settings);

    /// This class is not copyable.
    p2p(const p2p&) = delete;
    void operator=(const p2p&) = delete;

    /// Ensure all threads are coalesced.
    virtual ~p2p();

    // Start/Run sequences.
    // ------------------------------------------------------------------------

    /// Invoke startup and seeding sequence, call from constructing thread.
    virtual void start(result_handler handler);

    /// Synchronize the blockchain and then begin long running sessions,
    /// call from start result handler. Call base method to skip sync.
    virtual void run(result_handler handler);

    // Shutdown.
    // ------------------------------------------------------------------------

    /// Idempotent call to signal work stop, start may be reinvoked after.
    /// Returns the result of file save operation.
    virtual bool stop();

    /// Blocking call to coalesce all work and then terminate all threads.
    /// Call from thread that constructed this class, or don't call at all.
    /// This calls stop, and start may be reinvoked after calling this.
    virtual bool close();

    // Properties.
    // ------------------------------------------------------------------------

    /// Network configuration settings.
    virtual const settings& network_settings() const;

    /// Return the current block height.
    virtual size_t height() const;

    /// Set the current block height, for use in version messages.
    virtual void set_height(size_t value);

    /// Determine if the network is stopped.
    virtual bool stopped() const;

    /// Return a reference to the network threadpool.
    virtual threadpool& thread_pool();

    // Subscriptions.
    // ------------------------------------------------------------------------

    /// Subscribe to connection creation events.
    virtual void subscribe_connection(connect_handler handler);

    /// Subscribe to service stop event.
    virtual void subscribe_stop(result_handler handler);

    // Manual connections.
    // ----------------------------------------------------------------------------

    /// Maintain a connection to hostname:port.
    virtual void connect(const config::endpoint& peer);

    /// Maintain a connection to hostname:port.
    virtual void connect(const std::string& hostname, uint16_t port);

    /// Maintain a connection to hostname:port.
    /// The callback is invoked by the first connection creation only.
    virtual void connect(const std::string& hostname, uint16_t port,
        channel_handler handler);

    // Connections collection.
    // ------------------------------------------------------------------------

    /// Determine if there exists a connection to the address.
    virtual void connected(const address& address, truth_handler handler);

    /// Store a connection.
    virtual void store(channel::ptr channel, result_handler handler);

    /// Remove a connection.
    virtual void remove(channel::ptr channel, result_handler handler);

    /// Get the number of connections.
    virtual void connected_count(count_handler handler);

    // Hosts collection.
    // ------------------------------------------------------------------------

    /// Get a randomly-selected address.
    virtual void fetch_address(const config::authority::list& excluded_list, address_handler handler);

    ///Get authority of all connections
    config::authority::list authority_list();

    /// Store an address.
    virtual void store(const address& address, result_handler handler);

    /// Store a collection of addresses.
    virtual void store(const address::list& addresses, result_handler handler);

    /// Remove an address.
    virtual void remove(const address& address, result_handler handler);

    /// Get the number of addresses.
    virtual void address_count(count_handler handler);

    address::list address_list();

protected:

    /// Attach a session to the network, caller must start the session.
    template <class Session, typename... Args>
    typename Session::ptr attach(Args&&... args)
    {
        return std::make_shared<Session>(*this, std::forward<Args>(args)...);
    }

    /// Override to attach specialized sessions.
    virtual session_seed::ptr attach_seed_session();
    virtual session_manual::ptr attach_manual_session();
    virtual session_inbound::ptr attach_inbound_session();
    virtual session_outbound::ptr attach_outbound_session();

private:
    void handle_manual_started(const code& ec, result_handler handler);
    void handle_inbound_started(const code& ec, result_handler handler);
    void handle_hosts_loaded(const code& ec, result_handler handler);
    void handle_hosts_saved(const code& ec, result_handler handler);
    void handle_new_connection(const code& ec, channel::ptr channel,
        result_handler handler);

    void handle_started(const code& ec, result_handler handler);
    void handle_running(const code& ec, result_handler handler);

    const settings& settings_;

    // These are thread safe.
    std::atomic<bool> stopped_;
    std::atomic<size_t> height_;
    bc::atomic<session_manual::ptr> manual_;
    threadpool threadpool_;
    hosts::ptr hosts_;
    connections::ptr connections_;
    stop_subscriber::ptr stop_subscriber_;
    channel_subscriber::ptr channel_subscriber_;
};

} // namespace network
} // namespace libbitcoin

#endif
