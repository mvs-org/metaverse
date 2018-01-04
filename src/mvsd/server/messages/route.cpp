/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
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
#include <metaverse/server/messages/route.hpp>

#include <string>
#include <boost/functional/hash_fwd.hpp>
#include <metaverse/bitcoin.hpp>

namespace libbitcoin {
namespace server {

route::route()
  : secure(false), delimited(false)
{
}

std::string route::display() const
{
    return "[" + encode_base16(address1) +
        /*":" + encode_base16(address2) +*/ "]";
}

bool route::operator==(const route& other) const
{
    return secure == other.secure && /*delimited == other.delimited &&*/
        address1 == other.address1 /*&& address2 == other.address2*/;
}

} // namespace server
} // namespace libbitcoin
