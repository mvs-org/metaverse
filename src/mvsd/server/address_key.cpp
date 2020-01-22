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
#include <metaverse/server/utility/address_key.hpp>

#include <string>
#include <metaverse/bitcoin.hpp>
#include <metaverse/server/messages/route.hpp>

namespace libbitcoin {
namespace server {

address_key::address_key(const route& reply_to, const binary& prefix_filter)
  : reply_to_(reply_to), prefix_filter_(prefix_filter)
{
}

bool address_key::operator==(const address_key& other) const
{
    return reply_to_ == other.reply_to_ &&
        prefix_filter_ == other.prefix_filter_;
}

const route& address_key::reply_to() const
{
    return reply_to_;
}

const binary& address_key::prefix_filter() const
{
    return prefix_filter_;
}

} // namespace server
} // namespace libbitcoin
