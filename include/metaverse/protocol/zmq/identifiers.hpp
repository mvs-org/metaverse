/**
 * Copyright (c) 2011-2016 libbitcoin developers (see AUTHORS)
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
#ifndef MVS_PROTOCOL_ZMQ_IDENTIFIER_HPP
#define MVS_PROTOCOL_ZMQ_IDENTIFIER_HPP

#include <cstdint>
#include <vector>
#include <metaverse/protocol/define.hpp>

namespace libbitcoin {
namespace protocol {
namespace zmq {

/// A locally unique idenfitier for this socket.
typedef intptr_t identifier;

/// An indicator for a set of socket idenfitiers.
class BCP_API identifiers
{
    // Allow poller to push identifiers.
    friend class poller;

public:

    /// True if the result set contains no identifiers.
    bool empty() const;

    /// True if the result set contains the identifier.
    bool contains(identifier value) const;

protected:
    virtual void push(const void* socket);

    std::vector<identifier> ids_;
};

} // namespace zmq
} // namespace protocol
} // namespace libbitcoin

#endif
