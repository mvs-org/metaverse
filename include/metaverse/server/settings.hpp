/**
 * Copyright (c) 2011-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2020 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse-server.
 *
 * metaverse-server is free software: you can redistribute it and/or
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
#ifndef MVS_SERVER_SETTINGS_HPP
#define MVS_SERVER_SETTINGS_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>
#include <metaverse/node.hpp>
#include <metaverse/protocol.hpp>
#include <metaverse/server/define.hpp>

namespace libbitcoin {
namespace server {

/// Common database configuration settings, properties not thread safe.
class BCS_API settings
{
public:
    settings();
    settings(bc::settings context);

    /// Properties.
    uint16_t query_workers;
    uint32_t heartbeat_interval_seconds;
    uint32_t subscription_expiration_minutes;
    uint32_t subscription_limit;
    std::string mongoose_listen;
    std::string websocket_listen;
    std::string log_level;
    std::string rpc_version;
    bool administrator_required;
    bool secure_only;

    bool query_service_enabled;
    bool heartbeat_service_enabled;
    bool block_service_enabled;
    bool transaction_service_enabled;
    bool websocket_service_enabled;

    config::endpoint public_query_endpoint;
    config::endpoint public_heartbeat_endpoint;
    config::endpoint public_block_endpoint;
    config::endpoint public_transaction_endpoint;

    config::endpoint secure_query_endpoint;
    config::endpoint secure_heartbeat_endpoint;
    config::endpoint secure_block_endpoint;
    config::endpoint secure_transaction_endpoint;

    config::sodium server_private_key;
    config::sodium::list client_public_keys;
    config::authority::list client_addresses;

    std::vector<std::string> rpc_client_addresses;
    std::vector<std::string> allow_rpc_methods;
    std::vector<std::string> forbid_rpc_methods;

    /// Helpers.
    asio::duration heartbeat_interval() const;
    asio::duration subscription_expiration() const;
};

} // namespace server
} // namespace libbitcoin

#endif
