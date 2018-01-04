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
#ifndef MVS_SERVER_PROTOCOL_HPP
#define MVS_SERVER_PROTOCOL_HPP

#include <cstddef>
#include <metaverse/server/define.hpp>
#include <metaverse/server/messages/message.hpp>
#include <metaverse/server/server_node.hpp>

namespace libbitcoin {
namespace server {
    
/// Protocol interface.
/// Class and method names are published and mapped to the zeromq interface.
class BCS_API protocol
{
public:
    /// Broadcast a transaction to all connected peers.
    static void broadcast_transaction(server_node& node,
        const message& request, send_handler handler);

    /// Determine the count of all connected peers.
    static void total_connections(server_node& node,
        const message& request, send_handler handler);

private:
    static void handle_total_connections(size_t count,
        const message& request, send_handler handler);
};

} // namespace server
} // namespace libbitcoin

#endif
