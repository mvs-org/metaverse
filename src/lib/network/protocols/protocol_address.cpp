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
#include <bitcoin/network/protocols/protocol_address.hpp>

#include <functional>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/network/channel.hpp>
#include <bitcoin/network/p2p.hpp>
#include <bitcoin/network/protocols/protocol.hpp>
#include <bitcoin/network/protocols/protocol_events.hpp>

namespace libbitcoin {
namespace network {

#define NAME "address"
#define CLASS protocol_address

using namespace bc::message;
using namespace std::placeholders;

protocol_address::protocol_address(p2p& network, channel::ptr channel)
  : protocol_events(network, channel, NAME),
    network_(network),
    CONSTRUCT_TRACK(protocol_address)
{
}

// Start sequence.
// ----------------------------------------------------------------------------

void protocol_address::start()
{
    const auto& settings = network_.network_settings();

    // Must have a handler to capture a shared self pointer in stop subscriber.
    protocol_events::start(BIND1(handle_stop, _1));

    if (settings.self.port() != 0)
    {
        self_ = address({ { settings.self.to_network_address() } });
        SEND2(self_, handle_send, _1, self_.command);
    }

    // If we can't store addresses we don't ask for or handle them.
    if (settings.host_pool_capacity == 0)
        return;

    SUBSCRIBE2(address, handle_receive_address, _1, _2);
    SUBSCRIBE2(get_address, handle_receive_get_address, _1, _2);
    SEND2(get_address(), handle_send, _1, get_address::command);
}

// Protocol.
// ----------------------------------------------------------------------------

bool protocol_address::handle_receive_address(const code& ec,
    address::ptr message)
{
    if (stopped())
        return false;

    if (ec)
    {
        log::debug(LOG_NETWORK)
            << "Failure receiving address message from ["
            << authority() << "] " << ec.message();
        stop(ec);
        return false;
    }

    log::debug(LOG_NETWORK)
        << "Storing addresses from [" << authority() << "] ("
        << message->addresses.size() << ")";

    // TODO: manage timestamps (active channels are connected < 3 hours ago).
    network_.store(message->addresses, BIND1(handle_store_addresses, _1));

    // RESUBSCRIBE
    return true;
}

bool protocol_address::handle_receive_get_address(const code& ec,
    get_address::ptr message)
{
    if (stopped())
        return false;

    if (ec)
    {
        log::debug(LOG_NETWORK)
            << "Failure receiving get_address message from ["
            << authority() << "] " << ec.message();
        stop(ec);
        return false;
    }

    // TODO: allowing repeated queries can allow a channel to map our history.
    // TODO: pull active hosts from host cache (currently just resending self).
    // TODO: need to distort for privacy, don't send currently-connected peers.

    if (self_.addresses.empty())
        return false;

    log::debug(LOG_NETWORK)
        << "Sending addresses to [" << authority() << "] ("
        << self_.addresses.size() << ")";

    SEND2(self_, handle_send, _1, self_.command);

    // RESUBSCRIBE
    return true;
}

void protocol_address::handle_store_addresses(const code& ec)
{
    if (stopped())
        return;

    if (ec)
    {
        log::error(LOG_NETWORK)
            << "Failure storing addresses from [" << authority() << "] "
            << ec.message();
        stop(ec);
    }
}

void protocol_address::handle_stop(const code&)
{
    log::debug(LOG_NETWORK)
        << "Stopped addresss protocol";
}

} // namespace network
} // namespace libbitcoin
