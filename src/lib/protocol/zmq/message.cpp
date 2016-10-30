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
#include <bitcoin/protocol/zmq/message.hpp>

#include <string>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/protocol/zmq/frame.hpp>

namespace libbitcoin {
namespace protocol {
namespace zmq {

void message::enqueue()
{
    queue_.emplace(data_chunk{});
}

bool message::dequeue()
{
    if (queue_.empty())
        return false;

    queue_.pop();
    return true;
}

bool message::dequeue(uint32_t& value)
{
    if (queue_.empty())
        return false;

    const auto& front = queue_.front();

    if (front.size() == sizeof(uint32_t))
    {
        value = from_little_endian_unsafe<uint32_t>(front.begin());
        queue_.pop();
        return true;
    }

    queue_.pop();
    return false;
}

bool message::dequeue(data_chunk& value)
{
    if (queue_.empty())
        return false;

    value = dequeue_data();
    return true;
}

bool message::dequeue(std::string& value)
{
    if (queue_.empty())
        return false;

    value = dequeue_text();
    return true;
}

bool message::dequeue(hash_digest& value)
{
    if (queue_.empty())
        return false;

    const auto& front = queue_.front();

    if (front.size() == hash_size)
    {
        std::copy(front.begin(), front.end(), value.begin());
        queue_.pop();
        return true;
    }

    queue_.pop();
    return false;
}

data_chunk message::dequeue_data()
{
    if (queue_.empty())
        return{};

    const auto data = queue_.front();
    queue_.pop();
    return data;
}

std::string message::dequeue_text()
{
    if (queue_.empty())
        return{};

    const auto& front = queue_.front();
    const auto text = std::string(front.begin(), front.end());
    queue_.pop();
    return text;
}

void message::clear()
{
    while (!queue_.empty())
        queue_.pop();
}

bool message::empty() const
{
    return queue_.empty();
}

size_t message::size() const
{
    return queue_.size();
}

// Must be called on the socket thread.
code message::send(socket& socket)
{
    auto count = queue_.size();

    while (!queue_.empty())
    {
        frame frame(queue_.front());
        queue_.pop();
        const auto ec = frame.send(socket, --count == 0);

        if (ec)
            return ec;
    }

    return error::success;
}

// Must be called on the socket thread.
code message::receive(socket& socket)
{
    clear();
    auto done = false;

    while (!done)
    {
        frame frame;
        const auto ec = frame.receive(socket);

        if (ec)
            return ec;

        queue_.emplace(frame.payload());
        done = !frame.more();
    }

    return error::success;
}

} // namespace zmq
} // namespace protocol
} // namespace libbitcoin
