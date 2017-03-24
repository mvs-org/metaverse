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
#include <metaverse/lib/network/locked_socket.hpp>

#include <metaverse/lib/bitcoin.hpp>

namespace libbitcoin {
namespace network {

locked_socket::locked_socket(asio::socket& socket, upgrade_mutex& mutex)
  : socket_(socket),
    mutex_(mutex),
    CONSTRUCT_TRACK(locked_socket)
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock();
}

asio::socket& locked_socket::get()
{
    return socket_;
}

locked_socket::~locked_socket()
{
    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////
}

} // namespace network
} // namespace libbitcoin
