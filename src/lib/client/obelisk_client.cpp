/**
 * Copyright (c) 2011-2016 libbitcoin developers (see AUTHORS)
 *
 * This file is part of metaverse-client.
 *
 * metaverse-client is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/client/obelisk_client.hpp>

#include <algorithm>
#include <cstdint>
#include <thread>
#include <metaverse/protocol.hpp>

using namespace bc;
using namespace bc::config;
using namespace bc::protocol;
using namespace std::this_thread;

namespace libbitcoin {
namespace client {

static uint32_t to_milliseconds(uint16_t seconds)
{
    const auto milliseconds = static_cast<uint32_t>(seconds) * 1000;
    return std::min(milliseconds, max_uint32);
};

static const auto on_unknown = [](const std::string&){};

// Retries is overloaded as configuration for resends as well.
// Timeout is capped at ~ 25 days by signed/millsecond conversions.
obelisk_client::obelisk_client(uint16_t timeout_seconds, uint8_t retries)
  : socket_(context_, zmq::socket::role::dealer),
    stream_(socket_),
    retries_(retries),
    proxy(stream_, on_unknown, to_milliseconds(timeout_seconds), retries)
{
}

obelisk_client::obelisk_client(const connection_type& channel)
  : obelisk_client(channel.timeout_seconds, channel.retries)
{
}

bool obelisk_client::connect(const connection_type& channel)
{
    return connect(channel.server, channel.server_public_key,
        channel.client_private_key);
}

bool obelisk_client::connect(const endpoint& address)
{
    const auto host_address = address.to_string();

    for (auto attempt = 0; attempt < 1 + retries_; ++attempt)
    {
        if (socket_.connect(host_address).value() == error::success)
            return true;

        // Arbitrary delay between connection attempts.
        sleep_for(asio::milliseconds(100));
    }

    return false;
}

bool obelisk_client::connect(const endpoint& address,
    const sodium& server_public_key, const sodium& client_private_key)
{
    // Only apply the client (and server) key if server key is configured.
    if (server_public_key)
    {
        if (!socket_.set_curve_client(server_public_key))
            return false;

        // Generates arbitrary client keys if private key is not configured.
        if (!socket_.set_certificate({ client_private_key }))
            return false;
    }

    return connect(address);
}

// Used by fetch-* commands, fires reply, unknown or error handlers.
void obelisk_client::wait()
{
    zmq::poller poller;
    poller.add(socket_);
    auto delay = refresh();

    while (!empty() && poller.wait(delay).contains(socket_.id()))
    {
        stream_.read(*this);
        delay = refresh();
    }

    // Invoke error handlers for any still pending.
    clear(error::channel_timeout);
}

// Used by watch-* commands, fires registered update or unknown handlers.
void obelisk_client::monitor(uint32_t timeout_seconds)
{
    const auto deadline = asio::steady_clock::now() +
        asio::seconds(timeout_seconds);

    zmq::poller poller;
    poller.add(socket_);
    auto delay = remaining(deadline);

    while (poller.wait(delay).contains(socket_.id()))
    {
        stream_.read(*this);
        delay = remaining(deadline);
    }
}

} // namespace client
} // namespace libbitcoin
