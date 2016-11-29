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
#ifndef MVS_PROTOCOL_ZMQ_MESSAGE_HPP
#define MVS_PROTOCOL_ZMQ_MESSAGE_HPP

#include <string>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/protocol/zmq/socket.hpp>

namespace libbitcoin {
namespace protocol {
namespace zmq {

/// This class is not thread safe.
class BCP_API message
{
public:
    /// Add an empty message part to the outgoing message.
    void enqueue();

    /// Add an iterable message part to the outgoing message.
    template <typename Iterable>
    void enqueue(const Iterable& value)
    {
        queue_.emplace(to_chunk(value));
    }

    /// Add a message part to the outgoing message.
    template <typename Unsigned>
    void enqueue_little_endian(Unsigned value)
    {
        enqueue(to_little_endian<Unsigned>(value));
    }

    /// Remove a message part from the top of the queue, empty if empty queue.
    data_chunk dequeue_data();
    std::string dequeue_text();

    /// Remove a message part from the top of the queue, false if empty queue.
    bool dequeue();
    bool dequeue(uint32_t& value);
    bool dequeue(data_chunk& value);
    bool dequeue(std::string& value);
    bool dequeue(hash_digest& value);

    /// Clear the queue of message parts.
    void clear();

    /// True if the queue is empty.
    bool empty() const;

    /// The number of items on the queue.
    size_t size() const;

    /// Must be called on the socket thread.
    /// Send the message in parts. If a send fails the unsent parts remain.
    code send(socket& socket);

    /// Must be called on the socket thread.
    /// Receve a message (clears the queue first).
    code receive(socket& socket);

private:
    data_queue queue_;
};

} // namespace zmq
} // namespace protocol
} // namespace libbitcoin

#endif

