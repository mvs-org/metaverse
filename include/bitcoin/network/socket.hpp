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
#ifndef LIBBITCOIN_NETWORK_SOCKET_HPP
#define LIBBITCOIN_NETWORK_SOCKET_HPP

#include <bitcoin/bitcoin.hpp>
#include <bitcoin/network/define.hpp>
#include <bitcoin/network/locked_socket.hpp>

namespace libbitcoin {
namespace network {

/// A thread safe asio socket.
class BCT_API socket
  : public track<socket>
{
public:
    typedef std::shared_ptr<socket> ptr;

    /// Construct an instance.
    socket(threadpool& pool);

    /// This class is not copyable.
    socket(const socket&) = delete;
    void operator=(const socket&) = delete;

    /// Obtain an exclusive reference to the socket.
    locked_socket::ptr get_socket();

    /// Obtain the authority of the remote endpoint.
    config::authority get_authority() const;

    /// Close the contained socket.
    virtual void close();

private:
    asio::socket socket_;
    mutable upgrade_mutex mutex_;
};

} // namespace network
} // namespace libbitcoin

#endif
