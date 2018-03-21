/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
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
#ifndef MVS_NETWORK_PROXY_HPP
#define MVS_NETWORK_PROXY_HPP

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/const_buffer.hpp>
#include <metaverse/network/define.hpp>
#include <metaverse/network/message_subscriber.hpp>
#include <metaverse/network/socket.hpp>
#include <metaverse/bitcoin/utility/dispatcher.hpp>
#include <boost/thread.hpp>

namespace libbitcoin {
namespace network {

/// Manages all socket communication, thread safe.
class BCT_API proxy
  : public enable_shared_from_base<proxy>
{
public:
    typedef std::shared_ptr<proxy> ptr;
    typedef std::function<void()> completion_handler;
    typedef std::function<void(const code&)> result_handler;
    typedef subscriber<const code&> stop_subscriber;
    typedef resubscriber<const code&, const std::string&, const_buffer,
        result_handler> send_subscriber;
    using request_callback = std::function<void()>;

    /// Construct an instance.
    proxy(threadpool& pool, socket::ptr socket, uint32_t protocol_magic,
        uint32_t protocol_version);

    /// Validate proxy stopped.
    ~proxy();

    /// This class is not copyable.
    proxy(const proxy&) = delete;
    void operator=(const proxy&) = delete;

    /// Send a message on the socket.
    template <class Message>
    void send(const Message& message, result_handler handler)
    {
        const auto buffer = const_buffer(message::serialize(protocol_version_,
            message, protocol_magic_));
        do_send(message.command, buffer, handler);
    }

    /// Subscribe to messages of the specified type on the socket.
    template <class Message>
    void subscribe(message_handler<Message>&& handler)
    {
        auto stopped = std::forward<message_handler<Message>>(handler);
        message_subscriber_.subscribe<Message>(stopped);
    }

    /// Subscribe to the stop event.
    virtual void subscribe_stop(result_handler handler);

    /// Get the authority of the far end of this socket.
    virtual const config::authority& authority() const;

    /// Get the p2p protocol version object of the peer.
    virtual message::version version() const;

    /// Save the p2p protocol version object of the peer.
    virtual void set_version(message::version::ptr value);

    uint32_t peer_start_height() { return peer_version_message_.load() ? peer_version_message_.load()->start_height : 0; }

    /// Read messages from this socket.
    virtual void start(result_handler handler);

    /// Stop reading or sending messages on this socket.
    virtual void stop(const code& ec);

    void dispatch();

    static bool blacklisted(const config::authority&);
    static bool manualbanned(const config::authority& authority);
    static void manual_ban(const config::authority&);
    static void manual_unban(const config::authority&);

    virtual bool misbehaving(int32_t howmuch);

    virtual bool stopped() const;
protected:
    virtual void handle_activity() = 0;
    virtual void handle_stopping() = 0;

private:
    typedef byte_source<data_chunk> payload_source;
    typedef boost::iostreams::stream<payload_source> payload_stream;

    static config::authority authority_factory(socket::ptr socket);

    void do_close();
    void stop(const boost_code& ec);

    void read_heading();
    void handle_read_heading(const boost_code& ec, size_t payload_size);

    void read_payload(const message::heading& head);
    void handle_read_payload(const boost_code& ec, size_t,
        const message::heading& head);

    void do_send(const std::string& command, const_buffer buffer,
        result_handler handler);
    void handle_send(const boost_code& ec, const_buffer buffer,
        result_handler handler);

    void handle_request(data_chunk payload_buffer, uint32_t protocol_version_, message::heading head, size_t payload_size);

    const uint32_t protocol_magic_;
    const uint32_t protocol_version_;
    const config::authority authority_;

    // These are protected by sequential ordering.
    data_chunk heading_buffer_;
    data_chunk payload_buffer_;

    dispatcher dispatch_;

    // These are thread safe.
    socket::ptr socket_;
    std::atomic<bool> stopped_;
    std::atomic<uint32_t> peer_protocol_version_;
    bc::atomic<message::version::ptr> peer_version_message_;
    message_subscriber message_subscriber_;
    stop_subscriber::ptr stop_subscriber_;
    std::queue<request_callback> outbound_queue_;
    std::atomic_bool has_sent_;

    std::atomic_int misbehaving_;
    static boost::detail::spinlock spinlock_;
    static std::map<config::authority, int64_t> banned_;

    static boost::detail::spinlock manual_banned_spinlock_;
    static std::list<config::authority> manual_banned_;
};

} // namespace network
} // namespace libbitcoin

#endif

