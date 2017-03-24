/**
 * Copyright (c) 2011-2016 libbitcoin developers (see AUTHORS)
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
#ifndef MVS_LOCKED_SOCKET_HPP
#define MVS_LOCKED_SOCKET_HPP

#include <memory>
#include <metaverse/lib/bitcoin.hpp>
#include <metaverse/lib/network/define.hpp>

namespace libbitcoin {
namespace network {

/// A wrapper to lock the socket until out of scope.
class BCT_API locked_socket
  : track<locked_socket>
{
public:
    typedef std::shared_ptr<locked_socket> ptr;

    locked_socket(asio::socket& socket, upgrade_mutex& mutex);
    ~locked_socket();

    /// Obtain exclusive reference to the socket.
    /// The wrapper must be kept in scope while this reference is in use.
    asio::socket& get();

private:
    asio::socket& socket_;
    upgrade_mutex& mutex_;
};

} // namespace network
} // namespace libbitcoin

#endif
