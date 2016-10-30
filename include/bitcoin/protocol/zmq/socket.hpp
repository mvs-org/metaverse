/**
 * Copyright (c) 2011-2016 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin-protocol.
 *
 * libbitcoin-protocol is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
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
#ifndef LIBBITCOIN_PROTOCOL_ZMQ_SOCKET_HPP
#define LIBBITCOIN_PROTOCOL_ZMQ_SOCKET_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <bitcoin/protocol/define.hpp>
#include <bitcoin/protocol/zmq/certificate.hpp>
#include <bitcoin/protocol/zmq/context.hpp>
#include <bitcoin/protocol/zmq/identifiers.hpp>

namespace libbitcoin {
namespace protocol {
namespace zmq {

class message;
class authenticator;

/// This class is thread safe except as noted.
/// Because the socket is only set on construct, sockets are not restartable.
class BCP_API socket
  : public enable_shared_from_base<socket>
{
public:
    /// The full set of socket roles defined by zeromq.
    enum class role
    {
        pair,
        publisher,
        subscriber,
        requester,
        replier,
        dealer,
        router,
        puller,
        pusher,
        extended_publisher,
        extended_subscriber,
        streamer
    };

    /// A shared socket pointer.
    typedef std::shared_ptr<socket> ptr;

    /// Construct a socket from an existing zeromq socket.
    socket(void* zmq_socket);

    /// Construct a socket of the given context and role.
    socket(context& context, role socket_role);

    /// This class is not copyable.
    socket(const socket&) = delete;
    void operator=(const socket&) = delete;

    /// Close the socket.
    /// The object must be destroyed on the socket thread if not stopped.
    virtual ~socket();

    /// This must be called on the socket thread.
    /// Close the socket (optional, must close or destroy before context stop).
    virtual bool stop();

    /// True if the socket is valid.
    operator const bool() const;

    /// The underlying zeromq socket.
    void* self();

    /// An opaue locally unique idenfier, valid after stop.
    identifier id() const;

    /// This must be called on the socket thread.
    /// Bind the socket to the specified local address.
    code bind(const config::endpoint& address);

    /// This must be called on the socket thread.
    /// Connect the socket to the specified remote address.
    code connect(const config::endpoint& address);

    /// This must be called on the socket thread.
    /// Sets the domain for ZAP (ZMQ RFC 27) authentication.
    bool set_authentication_domain(const std::string& domain);

    /// This must be called on the socket thread.
    /// Configure the socket as a curve server (also set the secrety key).
    bool set_curve_server();

    /// This must be called on the socket thread.
    /// Configure the socket as client to the curve server.
    bool set_curve_client(const config::sodium& server_public_key);

    /// This must be called on the socket thread.
    /// Apply the specified public key to the socket.
    bool set_public_key(const config::sodium& key);

    /// This must be called on the socket thread.
    /// Apply the specified private key to the socket.
    bool set_private_key(const config::sodium& key);

    /// This must be called on the socket thread.
    /// Apply the keys of the specified certificate to the socket.
    bool set_certificate(const certificate& certificate);

    /// Send a message on this socket.
    code send(message& packet);

    /// Receive a message from this socket.
    code receive(message& packet);

private:
    static int to_socket_type(role socket_role);

    bool set(int32_t option, int32_t value);
    bool set(int32_t option, const std::string& value);

    // This is protected by mutex.
    void* self_;
    mutable shared_mutex mutex_;

    const identifier identifier_;
    const int32_t send_buffer_;
    const int32_t receive_buffer_;
};

} // namespace zmq
} // namespace protocol
} // namespace libbitcoin

#endif
