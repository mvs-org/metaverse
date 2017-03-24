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
#ifndef MVS_NETWORK_PROTOCOL_PING_HPP
#define MVS_NETWORK_PROTOCOL_PING_HPP

#include <cstdint>
#include <memory>
#include <metaverse/lib/bitcoin.hpp>
#include <metaverse/lib/network/channel.hpp>
#include <metaverse/lib/network/define.hpp>
#include <metaverse/lib/network/protocols/protocol_timer.hpp>
#include <metaverse/lib/network/settings.hpp>

namespace libbitcoin {
namespace network {

class p2p;

/**
 * Ping-pong protocol.
 * Attach this to a channel immediately following handshake completion.
 */
class BCT_API protocol_ping
  : public protocol_timer, track<protocol_ping>
{
public:
    typedef std::shared_ptr<protocol_ping> ptr;
    using result_handler = std::function<void(const code&)>;
    using ready_handler = std::function<void()>;

    /**
     * Construct a ping protocol instance.
     * @param[in]  network   The network interface.
     * @param[in]  channel   The channel on which to start the protocol.
     */
    protocol_ping(p2p& network, channel::ptr channel);

    /**
     * Start the protocol.
     */
    virtual void start(result_handler handler);

    void handle_or_not(uint64_t nonce);

private:
    void send_ping(const code& ec);
    void test_call_handler(const code& ec);
    bool handle_receive_ping(const code& ec, message::ping::ptr message);
    bool handle_receive_pong(const code& ec, message::pong::ptr message,
        uint64_t nonce);

    const settings& settings_;
    result_handler result_handler_;
    shared_mutex mutex_;
};

} // namespace network
} // namespace libbitcoin

#endif
