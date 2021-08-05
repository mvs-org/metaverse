/**
 * Copyright (c) 2011-2021 libbitcoin developers (see AUTHORS)
 *
 * This file is part of metaverse-protocol.
 *
 * metaverse-protocol is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
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
#include <metaverse/protocol/zmq/socket.hpp>

#include <algorithm>
#include <metaverse/protocol/zmq/identifiers.hpp>

namespace libbitcoin {
namespace protocol {
namespace zmq {

bool identifiers::empty() const
{
    return ids_.empty();
}

bool identifiers::contains(identifier value) const
{
    return std::find(ids_.begin(), ids_.end(), value) != ids_.end();
}

void identifiers::push(const void* socket)
{
    const auto value = reinterpret_cast<identifier>(socket);
    ids_.push_back(value);
}

} // namespace zmq
} // namespace protocol
} // namespace libbitcoin
