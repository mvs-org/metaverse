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
#ifndef MVS_NETWORK_PROTOCOL_SEED_HPP
#define MVS_NETWORK_PROTOCOL_SEED_HPP

#include <memory>
#include <metaverse/lib/bitcoin.hpp>
#include <metaverse/lib/network/channel.hpp>
#include <metaverse/lib/network/define.hpp>
#include <metaverse/lib/network/protocols/protocol_timer.hpp>

namespace libbitcoin {
namespace network {

class p2p;

/**
 * Seeding protocol.
 * Attach this to a channel immediately following seed handshake completion.
 */
class BCT_API protocol_seed
  : public protocol_timer, track<protocol_seed>
{
public:
    typedef std::shared_ptr<protocol_seed> ptr;

    /**
     * Construct a seed protocol instance.
     * @param[in]  network   The network interface.
     * @param[in]  channel   The channel on which to start the protocol.
     */
    protocol_seed(p2p& network, channel::ptr channel);

    /**
     * Start the protocol.
     * @param[in]  handler   Invoked upon stop or complete.
     */
    virtual void start(event_handler handler);

private:
    void send_own_address(const settings& settings);

    void handle_send_address(const code& ec);
    void handle_send_get_address(const code& ec);
    void handle_store_addresses(const code& ec);
    void handle_seeding_complete(const code& ec, event_handler handler);

    bool handle_receive_address(const code& ec, message::address::ptr address);
    ////bool handle_receive_get_address(const code& ec,
    ////    message::get_address::ptr message);

    p2p& network_;
    const config::authority self_;
};

} // namespace network
} // namespace libbitcoin

#endif
