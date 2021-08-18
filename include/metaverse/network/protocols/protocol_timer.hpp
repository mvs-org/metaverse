/**
 * Copyright (c) 2011-2021 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2021 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse.
 *
 * metaverse is free software: you can redistribute it and/or modify
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
#ifndef MVS_NETWORK_PROTOCOL_TIMER_HPP
#define MVS_NETWORK_PROTOCOL_TIMER_HPP

#include <string>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/channel.hpp>
#include <metaverse/network/define.hpp>
#include <metaverse/network/protocols/protocol_events.hpp>

namespace libbitcoin {
namespace network {

class p2p;

/**
 * Base class for timed protocol implementation.
 */
class BCT_API protocol_timer
  : public protocol_events
{
protected:

    /**
     * Construct a timed protocol instance.
     * @param[in]  network   The network interface.
     * @param[in]  channel    The channel on which to start the protocol.
     * @param[in]  perpetual  Set for automatic timer reset unless stopped.
     * @param[in]  name       The instance name for logging purposes.
     */
    protocol_timer(p2p& network, channel::ptr channel, bool perpetual,
        const std::string& name);

    /**
     * Define the event handler and start the protocol and timer.
     * The timer is automatically canceled on stop (only).
     * The timer is suspended while the handler is executing.
     * @param[in]  timeout  The timer period (not automatically reset).
     * @param[in]  handler  Invoke automatically on stop and timer events.
     */
    void start(const asio::duration& timeout, event_handler handler);

protected:
    void reset_timer();

private:
    void handle_timer(const code& ec);
    void handle_notify(const code& ec, event_handler handler);

    const bool perpetual_;
    deadline::ptr timer_;
};

} // namespace network
} // namespace libbitcoin

#endif
