/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin-server.
 *
 * libbitcoin-server is free software: you can redistribute it and/or
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
#include <bitcoin/server/utility/authenticator.hpp>

#include <string>
#include <bitcoin/protocol.hpp>
#include <bitcoin/server/configuration.hpp>
#include <bitcoin/server/server_node.hpp>

namespace libbitcoin {
namespace server {

using namespace bc::protocol;

authenticator::authenticator(server_node& node)
  : zmq::authenticator(node.thread_pool())
{
    const auto& settings = node.server_settings();

    set_private_key(settings.server_private_key);

    for (const auto& address: settings.client_addresses)
    {
        log::debug(LOG_SERVER)
            << "Allow client address [" << address
            << (address.port() == 0 ? ":*" : "") << "]";

        allow(address);
    }

    for (const auto& public_key: settings.client_public_keys)
    {
        log::debug(LOG_SERVER)
            << "Allow client public key [" << public_key << "]";

        allow(public_key);
    }
}

bool authenticator::apply(zmq::socket& socket, const std::string& domain,
    bool secure)
{
    // This will fail if there are client keys but no server key.
    if (!zmq::authenticator::apply(socket, domain, secure))
    {
        log::error(LOG_SERVER)
            << "Failed to apply authentication to socket [" << domain << "]";
        return false;
    }

    if (secure)
    {
        log::debug(LOG_SERVER)
            << "Applied curve authentication to socket [" << domain << "]";
    }
    else
    {
        log::debug(LOG_SERVER)
            << "Applied address authentication to socket [" << domain << "]";
    }

    return true;
}

} // namespace server
} // namespace libbitcoin
