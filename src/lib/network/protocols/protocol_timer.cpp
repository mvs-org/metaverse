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
#include <bitcoin/network/protocols/protocol_timer.hpp>

#include <functional>
#include <memory>
#include <string>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/network/channel.hpp>
#include <bitcoin/network/p2p.hpp>
#include <bitcoin/network/protocols/protocol_events.hpp>

namespace libbitcoin {
namespace network {

#define CLASS protocol_timer
using namespace std::placeholders;

protocol_timer::protocol_timer(p2p& network, channel::ptr channel,
    bool perpetual, const std::string& name)
  : protocol_events(network, channel, name),
    perpetual_(perpetual)
{
}

// Start sequence.
// ----------------------------------------------------------------------------

// protected:
void protocol_timer::start(const asio::duration& timeout,
    event_handler handle_event)
{
    // The deadline timer is thread safe.
    timer_ = std::make_shared<deadline>(pool(), timeout);
    protocol_events::start(BIND2(handle_notify, _1, handle_event));
    reset_timer();
}

void protocol_timer::handle_notify(const code& ec, event_handler handler)
{
    if (ec == error::channel_stopped)
        timer_->stop();

    handler(ec);
}

// Timer.
// ----------------------------------------------------------------------------

// protected:
void protocol_timer::reset_timer()
{
    if (stopped())
        return;

    timer_->start(BIND1(handle_timer, _1));
}

void protocol_timer::handle_timer(const code& ec)
{
    if (stopped())
        return;

#if 0 //huge log genareted here
    log::debug(LOG_NETWORK)
        << "Fired protocol_" << name() << " timer on [" << authority() << "] "
        << ec.message();
#endif

    // The handler completes before the timer is reset.
    set_event(error::channel_timeout);

    // A perpetual timer resets itself until the channel is stopped.
    if (perpetual_)
        reset_timer();
}

} // namespace network
} // namespace libbitcoin
