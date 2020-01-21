/**
 * Copyright (c) 2019-2020 libbitcoin developers (see AUTHORS)
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
#include <metaverse/network/settings.hpp>

#include <metaverse/bitcoin.hpp>

namespace libbitcoin {
namespace network {

using namespace bc::asio;
using namespace bc::message;

// Common default values (no settings context).
settings::settings()
  : threads(16),
    protocol(version::level::maximum),
    inbound_connections(32),
    outbound_connections(8),
    manual_attempt_limit(0),
    connect_batch_size(5),
    connect_timeout_seconds(5),
    channel_handshake_seconds(30),
    channel_heartbeat_minutes(5),
    channel_inactivity_minutes(10),
    channel_expiration_minutes(1440),
    channel_germination_seconds(30),
    host_pool_capacity(1000),
    relay_transactions(true),
    enable_re_seeding(true),
    upnp_map_port(true),
    be_found(true),
    hosts_file("hosts.cache"),
    debug_file("debug.log"),
    error_file("error.log"),
    self(unspecified_network_address)
{
}

// Use push_back due to initializer_list bug:
// stackoverflow.com/a/20168627/1172329
settings::settings(bc::settings context)
  : settings()
{
    // Handle deviations from common defaults.
    switch (context)
    {
        case bc::settings::mainnet:
        {
            identifier = 0x4d53564d;
            inbound_port = 5251;

            // Seeds based on mvs.live/network/dns-servers
            seeds.reserve(6);
            seeds.push_back({ "main-asia.metaverse.live", 5251 });
            seeds.push_back({ "main-americas.metaverse.live", 5251 });
            seeds.push_back({ "main-europe.metaverse.live", 5251 });
            seeds.push_back({ "main-asia.mvs.live", 5251 });
            seeds.push_back({ "main-americas.mvs.live", 5251 });
            seeds.push_back({ "main-europe.mvs.live", 5251 });
            seeds.push_back({ "seed.getmvs.org", 5251 });
            break;
        }

        case bc::settings::testnet:
        {
            identifier = 0x73766d74;
            inbound_port = 15251;

            seeds.reserve(6);
            seeds.push_back({ "test-asia.metaverse.live", 15251 });
            seeds.push_back({ "test-americas.metaverse.live", 15251 });
            seeds.push_back({ "test-europe.metaverse.live", 15251 });
            seeds.push_back({ "test-asia.mvs.live", 15251 });
            seeds.push_back({ "test-americas.mvs.live", 15251 });
            seeds.push_back({ "test-europe.mvs.live", 15251 });
            break;
        }

        default:
        case bc::settings::none:
        {
        }
    }
}

duration settings::connect_timeout() const
{
    return seconds(connect_timeout_seconds);
}

duration settings::channel_handshake() const
{
    return seconds(channel_handshake_seconds);
}

duration settings::channel_heartbeat() const
{
    return minutes(channel_heartbeat_minutes);
}

duration settings::channel_inactivity() const
{
    return minutes(channel_inactivity_minutes);
}

duration settings::channel_expiration() const
{
    return minutes(channel_expiration_minutes);
}

duration settings::channel_germination() const
{
    return seconds(channel_germination_seconds);
}

} // namespace network
} // namespace libbitcoin
