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
#ifndef MVS_SERVER_ADDRESS_HPP
#define MVS_SERVER_ADDRESS_HPP

#include <bitcoin/server/define.hpp>
#include <bitcoin/server/messages/message.hpp>
#include <bitcoin/server/server_node.hpp>

namespace libbitcoin {
namespace server {

/// Address interface.
/// Class and method names are published and mapped to the zeromq interface.
class BCS_API address
{
public:
    /// Fetch the blockchain and transaction pool history of a payment address.
    static void fetch_history2(server_node& node,
        const message& request, send_handler handler);

    /// Alias for subscribe, preserved for backward compatability.
    static void renew(server_node& node, const message& request,
        send_handler handler);

    /// Subscribe to payment or stealth address notifications by prefix.
    static void subscribe(server_node& node, const message& request,
        send_handler handler);

    /// Subscribe to payment and stealth address notifications by prefix.
    static void subscribe2(server_node& node, const message& request,
        send_handler handler);

    /// Unsubscribe to payment and stealth address notifications by prefix.
    static void unsubscribe2(server_node& node, const message& request,
        send_handler handler);

private:
    static bool unwrap_subscribe_args(binary& prefix_filter,
        chain::subscribe_type& type, const message& request);

    static bool unwrap_subscribe2_args(binary& prefix_filter,
        const message& request);
};

} // namespace server
} // namespace libbitcoin

#endif
