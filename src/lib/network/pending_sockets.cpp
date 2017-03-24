/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
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
#include <metaverse/lib/network/pending_sockets.hpp>

#include <algorithm>
#include <metaverse/lib/bitcoin.hpp>
#include <metaverse/lib/network/proxy.hpp>
#include <metaverse/lib/network/socket.hpp>

namespace libbitcoin {
namespace network {

pending_sockets::pending_sockets()
{
}

pending_sockets::~pending_sockets()
{
    BITCOIN_ASSERT_MSG(sockets_.empty(), "Pending sockets not cleared.");
}

void pending_sockets::clear()
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    unique_lock lock(mutex_);

    // This will asynchronously invoke the handler of each pending connect.
    for (auto socket: sockets_)
        socket->close();

    sockets_.clear();
    ///////////////////////////////////////////////////////////////////////////
}

void pending_sockets::store(socket::ptr socket)
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    unique_lock lock(mutex_);

    // We could test the list for the socket, which should not exist.
    sockets_.push_back(socket);
    ///////////////////////////////////////////////////////////////////////////
}

void pending_sockets::remove(socket::ptr socket)
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_upgrade();

    auto it = std::find(sockets_.begin(), sockets_.end(), socket);
    const auto found = it != sockets_.end();

    // Clear can be called at any time, so the entry may not be found.
    if (found)
    {
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        mutex_.unlock_upgrade_and_lock();
        sockets_.erase(it);
        mutex_.unlock();
        //---------------------------------------------------------------------
        return;
    }

    mutex_.unlock_upgrade();
    ///////////////////////////////////////////////////////////////////////////
}

} // namespace network
} // namespace libbitcoin
