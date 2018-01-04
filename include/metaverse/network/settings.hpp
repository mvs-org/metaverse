/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse.
 *
 * metaverse is free software: you can redistribute it and/or
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
#ifndef MVS_NETWORK_SETTINGS_HPP
#define MVS_NETWORK_SETTINGS_HPP

#include <cstdint>
#include <boost/filesystem.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/define.hpp>

namespace libbitcoin {
namespace network {

/// Common database configuration settings, properties not thread safe.
class BCT_API settings
{
public:
    settings();
    settings(bc::settings context);

    /// Properties.
    uint32_t threads;
    uint32_t protocol;
    uint32_t identifier;
    uint16_t inbound_port;
    uint32_t inbound_connections;
    uint32_t outbound_connections;
    uint32_t manual_attempt_limit;
    uint32_t connect_batch_size;
    uint32_t connect_timeout_seconds;
    uint32_t channel_handshake_seconds;
    uint32_t channel_heartbeat_minutes;
    uint32_t channel_inactivity_minutes;
    uint32_t channel_expiration_minutes;
    uint32_t channel_germination_seconds;
    uint32_t host_pool_capacity;
    bool relay_transactions;
    boost::filesystem::path hosts_file;
    boost::filesystem::path debug_file;
    boost::filesystem::path error_file;
    config::authority self;
    config::authority::list blacklists;
    config::endpoint::list peers;
    config::endpoint::list seeds;

    /// Helpers.
    asio::duration connect_timeout() const;
    asio::duration channel_handshake() const;
    asio::duration channel_heartbeat() const;
    asio::duration channel_inactivity() const;
    asio::duration channel_expiration() const;
    asio::duration channel_germination() const;
};

} // namespace network
} // namespace libbitcoin

#endif
