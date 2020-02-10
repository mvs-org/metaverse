/**
 * Copyright (c) 2011-2020 libbitcoin developers (see AUTHORS)
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
#ifndef MVS_CLIENT_OBELISK_CLIENT_HPP
#define MVS_CLIENT_OBELISK_CLIENT_HPP

#include <cstdint>
#include <metaverse/protocol.hpp>
#include <metaverse/client/define.hpp>
#include <metaverse/client/proxy.hpp>
#include <metaverse/client/socket_stream.hpp>

namespace libbitcoin {
namespace client {

/// Structure used for passing connection settings for a server.
struct BCC_API connection_type
{
    uint8_t retries;
    uint16_t timeout_seconds;
    config::endpoint server;
    config::sodium server_public_key;
    config::sodium client_private_key;
};

/// Client proxy with session management.
class BCC_API obelisk_client
  : public proxy
{
public:
    /// Construct an instance of the client using timeout/retries from channel.
    obelisk_client(const connection_type& channel);

    /// Construct an instance of the client using the specified parameters.
    obelisk_client(uint16_t timeout_seconds, uint8_t retries);

    /// Connect to the specified endpoint.
    virtual bool connect(const config::endpoint& address);

    /// Connect to the specified endpoint using the provided keys.
    virtual bool connect(const config::endpoint& address,
        const bc::config::sodium& server_public_key,
        const bc::config::sodium& client_private_key);

    /// Connect to the specified endpoint using the provided channel config.
    virtual bool connect(const connection_type& channel);

    /// Wait for server to respond, until timeout.
    void wait();

    /// Monitor for subscription notifications, until timeout.
    void monitor(uint32_t timeout_seconds);

private:
    protocol::zmq::context context_;
    protocol::zmq::socket socket_;
    socket_stream stream_;
    const uint8_t retries_;
};

} // namespace client
} // namespace libbitcoin

#endif
