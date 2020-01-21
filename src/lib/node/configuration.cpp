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
#include <metaverse/node/configuration.hpp>

#include <cstddef>
#include <metaverse/blockchain.hpp>
#include <metaverse/network.hpp>

namespace libbitcoin {
namespace node {

// Construct with defaults derived from given context.
configuration::configuration(bc::settings context)
  : help(false),
    initchain(false),
    settings(false),
    version(false),
    daemon{false},
    use_testnet_rules{false},
    upnp_map_port{true},
    node(context),
    chain(context),
    database(context),
    network(context)
{
}

// Copy constructor.
configuration::configuration(const configuration& other)
  : help(other.help),
    initchain(other.initchain),
    settings(other.settings),
    version(other.version),
    daemon{other.daemon},
    use_testnet_rules{other.use_testnet_rules},
    upnp_map_port{other.upnp_map_port},
    file(other.file),
    node(other.node),
    chain(other.chain),
    database(other.database),
    network(other.network)
{
}

} // namespace node
} // namespace libbitcoin
