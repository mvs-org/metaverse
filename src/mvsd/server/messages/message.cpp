/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin-server.
 *
 * libbitcoin-server is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/lib/server/messages/message.hpp>

#include <cstdint>
#include <string>
#include <metaverse/lib/protocol.hpp>
#include <metaverse/lib/server/messages/route.hpp>

namespace libbitcoin {
namespace server {

using namespace bc::protocol;

// Protocol delimitation migration plan.
//-----------------------------------------------------------------------------
//    v1/v2 server: ROUTER, requires not delimited
//       v3 server: ROUTER, allows/echos delimited
// v1/v2/v3 client: DEALER (not delimited)
//-----------------------------------------------------------------------------
//       v4 server: ROUTER, requires delimited
//       v4 client: DEALER (delimited) or REQ
//-----------------------------------------------------------------------------

// Convert an error code to data for payload.
data_chunk message::to_bytes(const code& ec)
{
    return build_chunk(
    {
        to_little_endian(static_cast<uint32_t>(ec.value()))
    });
}

// Constructors.
//-------------------------------------------------------------------------

// Construct an empty message with security routing context.
message::message(bool secure)
{
    // For subscriptions, directs notifier to respond on secure endpoint.
    route_.secure = secure;
}

// Construct a response for the request (response code only).
message::message(const message& request, const code& ec)
  : message(request, to_bytes(ec))
{
}

// Construct a response for the request (response data with code).
message::message(const message& request, const data_chunk& data)
  : message(request.route(), request.command(), request.id(), data)
{
}

// Construct a response for the route (subscription code only).
message::message(const server::route& route, const std::string& command,
    uint32_t id, const code& ec)
  : message(route, command, id, to_bytes(ec))
{
}

// Construct a response for the route (subscription data with code).
message::message(const server::route& route, const std::string& command,
    uint32_t id, const data_chunk& data)
  : route_(route), command_(command), id_(id), data_(data)
{
}

// Properties.
//-------------------------------------------------------------------------

/// Arbitrary caller data (returned to caller for correlation).
uint32_t message::id() const
{
    return id_;
}

/// Serialized query or response (defined in relation to command).
const data_chunk& message::data() const
{
    return data_;
}

/// Query command (used for subscription, always returned to caller).
const std::string& message::command() const
{
    return command_;
}

/// The message route.
const server::route& message::route() const
{
    return route_;
}

// Transport.
//-------------------------------------------------------------------------

code message::receive(zmq::socket& socket)
{
    zmq::message message;
    auto ec = socket.receive(message);

    if (ec)
        return ec;
    
    if (message.size() < 5 || message.size() > 6)
        return error::bad_stream;

    // Decode the routing information (TODO: generalize in route).
    //-------------------------------------------------------------------------

    // Client is undelimited DEALER -> 2 addresses with no delimiter.
    // Client is REQ or delimited DEALER -> 2 addresses with delimiter.
    route_.address1 = message.dequeue_data();
    route_.address2 = message.dequeue_data();

    // In the reply we echo the delimited-ness of the original request.
    route_.delimited = message.size() == 4;

    if (route_.delimited)
        message.dequeue();

    // All libbitcoin queries and responses have these three frames.
    //-------------------------------------------------------------------------

    // Query command (returned to caller).
    command_ = message.dequeue_text();

    // Arbitrary caller data (returned to caller for correlation).
    if (!message.dequeue(id_))
        return error::bad_stream;

    // Serialized query.
    data_ = message.dequeue_data();

    return error::success;
}

code message::send(zmq::socket& socket)
{
    zmq::message message;

    // Encode the routing information (TODO: generalize in route).
    //-------------------------------------------------------------------------

    // Client is undelimited DEALER -> 2 addresses with no delimiter.
    // Client is REQ or delimited DEALER -> 2 addresses with delimiter.
    message.enqueue(route_.address1);
    message.enqueue(route_.address2);

    // In the reply we echo the delimited-ness of the original request.
    if (route_.delimited)
        message.enqueue();

    // All libbitcoin queries and responses have these three frames.
    //-------------------------------------------------------------------------
    message.enqueue(command_);
    message.enqueue_little_endian(id_);
    message.enqueue(data_);

    return socket.send(message);
}

} // namespace server
} // namespace libbitcoin
