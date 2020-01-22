/**
 * Copyright (c) 2011-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2020 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse-server.
 *
 * metaverse-server is free software: you can redistribute it and/or
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
#ifndef MVS_SERVER_MESSAGE
#define MVS_SERVER_MESSAGE

#include <cstdint>
#include <string>
#include <metaverse/protocol.hpp>
#include <metaverse/server/define.hpp>
#include <metaverse/server/messages/route.hpp>

namespace libbitcoin {
namespace server {

class BCS_API message
{
public:
    static data_chunk to_bytes(const code& ec);

    //// Construct an empty message with security routing context.
    message(bool secure);

    //// Construct a response for the request (code only).
    message(const message& request, const code& ec);

    //// Construct a response for the request (data with code).
    message(const message& request, const data_chunk& data);

    //// Construct a response for the route (subscription code only).
    message(const server::route& route, const std::string& command,
        uint32_t id, const code& ec);

    //// Construct a response for the route (subscription data with code).
    message(const server::route& route, const std::string& command,
        uint32_t id, const data_chunk& data);

    /// Arbitrary caller data (returned to caller for correlation).
    uint32_t id() const;

    /// Serialized query or response (defined in relation to command).
    const data_chunk& data() const;

    /// Query command (used for subscription, always returned to caller).
    const std::string& command() const;

    /// The message route.
    const server::route& route() const;

    /// Receive a message via the socket.
    code receive(bc::protocol::zmq::socket& socket);

    /// Send the message via the socket.
    code send(bc::protocol::zmq::socket& socket);

private:
    uint32_t id_;
    data_chunk data_;
    server::route route_;
    std::string command_;
};

typedef std::function<void(message&&)> send_handler;

} // namespace server
} // namespace libbitcoin

#endif
