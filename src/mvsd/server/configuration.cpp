/**
 * Copyright (c) 2011-2021 libbitcoin developers (see AUTHORS)
 *
 * This file is part of metaverse.
 *
 * metaverse-server is free software: you can redistribute it and/or modify
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
#include <metaverse/server/configuration.hpp>

#include <metaverse/server/settings.hpp>

namespace libbitcoin {
namespace server {

// Construct with defaults derived from given context.
configuration::configuration(bc::settings context)
  : node::configuration(context),
    server(context)
{
}

// Copy constructor.
configuration::configuration(const configuration& other)
  : node::configuration(other),
    server(other.server)
{
}

} // namespace server
} // namespace libbitcoin
