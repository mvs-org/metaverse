/**
 * Copyright (c) 2011-2016 libbitcoin developers (see AUTHORS)
 *
 * This file is part of metaverse-protocol.
 *
 * metaverse-protocol is free software: you can redistribute it and/or
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
#ifndef MVS_PROTOCOL_ZMQ_FRAME_HPP
#define MVS_PROTOCOL_ZMQ_FRAME_HPP

#include <memory>
#include <metaverse/bitcoin.hpp>
#include <metaverse/protocol/define.hpp>
#include <metaverse/protocol/zmq/socket.hpp>
#include <metaverse/protocol/zmq/zeromq.hpp>

namespace libbitcoin {
namespace protocol {
namespace zmq {

/// This class is not thread safe.
class BCP_API frame
  : public enable_shared_from_base<frame>
{
public:
    /// A shared frame pointer.
    typedef std::shared_ptr<frame> ptr;

    /// Construct a frame with no payload (for receiving).
    frame();

    /// Construct a frame with the specified payload (for sending).
    frame(const data_chunk& data);

    /// This class is not copyable.
    frame(const frame&) = delete;
    void operator=(const frame&) = delete;

    /// Free the frame's allocated memory.
    virtual ~frame();

    /// True if the construction was successful.
    operator const bool() const;

    /// True if there is more data to receive.
    bool more() const;

    /// The initialized or received payload of the frame.
    data_chunk payload();

    /// Must be called on the socket thread.
    /// Receive a frame on the socket.
    code receive(socket& socket);

    /// Must be called on the socket thread.
    /// Send a frame on the socket.
    code send(socket& socket, bool more);

private:
    // zmq_msg_t alias, keeps zmq.h out of our headers.
    typedef union
    {
        unsigned char alignment[64];
        void* pointer;
    } zmq_msg;

    static bool initialize(zmq_msg& message, const data_chunk& data);

    bool set_more(socket& socket);
    bool destroy();

    bool more_;
    const bool valid_;
    zmq_msg message_;
};

} // namespace zmq
} // namespace protocol
} // namespace libbitcoin

#endif

