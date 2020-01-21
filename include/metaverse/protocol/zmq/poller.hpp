/**
 * Copyright (c) 2011-2020 libbitcoin developers (see AUTHORS)
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
#ifndef MVS_PROTOCOL_ZMQ_POLLER_HPP
#define MVS_PROTOCOL_ZMQ_POLLER_HPP

#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>
#include <metaverse/protocol/define.hpp>
#include <metaverse/protocol/zmq/identifiers.hpp>
#include <metaverse/protocol/zmq/socket.hpp>

namespace libbitcoin {
namespace protocol {
namespace zmq {

/// This class is thread safe except as noted.
class BCP_API poller
  : public enable_shared_from_base<poller>
{
public:
    /// A shared poller pointer.
    typedef std::shared_ptr<poller> ptr;

    /// Construct an empty poller (sockets must be added).
    poller();

    /// This class is not copyable.
    poller(const poller&) = delete;
    void operator=(const poller&) = delete;

    /// True if the timeout occurred.
    bool expired() const;

    /// True if the connection is closed.
    bool terminated() const;

    /// Add a socket to be polled.
    void add(socket& sock);

    /// Remove all sockets from the poller.
    void clear();

    /// This must be called on the socket thread.
    /// Wait one second for any socket to receive.
    identifiers wait();

    /// This must be called on the socket thread.
    /// Wait specified time for any socket to receive, -1 is forever.
    identifiers wait(int32_t timeout_milliseconds);

private:
    // zmq_pollitem_t alias, keeps zmq.h out of our headers.
    typedef struct
    {
        void* socket;
        file_descriptor fd;
        short events;
        short revents;
    } zmq_pollitem;

    typedef std::vector<zmq_pollitem> pollers;

    // These values are protected by mutex.
    bool expired_;
    bool terminated_;
    pollers pollers_;
    mutable shared_mutex mutex_;
};

} // namespace zmq
} // namespace protocol
} // namespace libbitcoin

#endif
