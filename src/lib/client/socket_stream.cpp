/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin-client.
 *
 * libbitcoin-client is free software: you can redistribute it and/or
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
#include <metaverse/lib/client/socket_stream.hpp>

#include <cstdint>
#include <metaverse/lib/protocol.hpp>

namespace libbitcoin {
namespace client {

using namespace bc::protocol;

socket_stream::socket_stream(zmq::socket& socket)
  : socket_(socket)
{
}

zmq::socket& socket_stream::socket()
{
    return socket_;
}

// Stream interface, not utilized on this class.
int32_t socket_stream::refresh()
{
    return 0;
}

// TODO: optimize by passing the internal type of the message object.
// Receieve a message from this socket onto the stream parameter.
bool socket_stream::read(stream& stream)
{
    data_stack data;
    zmq::message message;

    if (socket_.receive(message) != error::success)
        return false;

    // Copy the message to a data stack.
    while (!message.empty())
        data.push_back(message.dequeue_data());

    return stream.write(data);
}

////bool socket_stream::read(std::shared_ptr<response_stream> stream)
////{
////    if (!stream)
////        return false;
////
////    response_message message;
////
////    if (socket_.receive(message) != error::success)
////        return false;
////
////    auto response = message.get_response();
////    return response && (stream->write(response) == error::success);
////}

// TODO: optimize by passing the internal type of the message object.
// Send a message built from the stack parameter to this socket.
bool socket_stream::write(const data_stack& data)
{
    zmq::message message;

    // Copy the data stack to a message.
    for (const auto& chunk: data)
        message.enqueue(chunk);

    return socket_.send(message) == error::success;
}

////bool socket_stream::write(const std::shared_ptr<request>& request)
////{
////    if (!request)
////        return false;
////
////    request_message message;
////    message.set_request(request);
////    return socket_.send(message) == error::success;
////}

} // namespace client
} // namespace libbitcoin