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
#ifndef MVS_NETWORK_ACCEPTOR_HPP
#define MVS_NETWORK_ACCEPTOR_HPP

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <metaverse/lib/bitcoin.hpp>
#include <metaverse/lib/network/channel.hpp>
#include <metaverse/lib/network/define.hpp>
#include <metaverse/lib/network/settings.hpp>
#include <metaverse/lib/network/socket.hpp>

namespace libbitcoin {
namespace network {

/// Create inbound socket connections, thread and lock safe.
class BCT_API acceptor
  : public enable_shared_from_base<acceptor>, track<acceptor>
{
public:
    typedef std::shared_ptr<acceptor> ptr;
    typedef std::function<void(const code&)> result_handler;
    typedef std::function<void(const code&, channel::ptr)> accept_handler;

    /// Construct an instance.
    acceptor(threadpool& pool, const settings& settings);

    /// Validate acceptor stopped.
    ~acceptor();

    /// This class is not copyable.
    acceptor(const acceptor&) = delete;
    void operator=(const acceptor&) = delete;

    /// Start the listener on the specified port.
    virtual void listen(uint16_t port, result_handler handler);

    /// Accept the next connection available, until canceled.
    virtual void accept(accept_handler handler);

    /// Cancel the listener and all outstanding accept attempts.
    virtual void stop();

private:
    code safe_listen(uint16_t port);
    void safe_accept(socket::ptr socket, accept_handler handler);
    std::shared_ptr<channel> new_channel(socket::ptr socket);
    void handle_accept(const boost_code& ec, socket::ptr socket,
        accept_handler handler);

    threadpool& pool_;
    const settings& settings_;
    dispatcher dispatch_;
    asio::acceptor_ptr acceptor_;
    mutable shared_mutex mutex_;
};

} // namespace network
} // namespace libbitcoin

#endif
