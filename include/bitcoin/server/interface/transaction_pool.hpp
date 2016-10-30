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
#ifndef LIBBITCOIN_SERVER_TRANSACTION_POOL_HPP
#define LIBBITCOIN_SERVER_TRANSACTION_POOL_HPP

#include <bitcoin/bitcoin.hpp>
#include <bitcoin/server/define.hpp>
#include <bitcoin/server/messages/message.hpp>
#include <bitcoin/server/server_node.hpp>

namespace libbitcoin {
namespace server {

/// Transaction pool interface.
/// Class and method names are published and mapped to the zeromq interface.
class BCS_API transaction_pool
{
public:
    /// Fetch a transaction from the transaction pool (only), by its hash.
    static void fetch_transaction(server_node& node, const message& request,
        send_handler handler);

    /// Broadcast a transaction with penetration subscription.
    static void broadcast(server_node& node, const message& request,
        send_handler handler);

    /// Validate a transaction against the transaction pool and blockchain.
    static void validate(server_node& node, const message& request,
        send_handler handler);

private:
    static void handle_validated(const code& ec,
        bc::message::transaction_message::ptr tx,
        const chain::point::indexes& unconfirmed, const message& request,
        send_handler handler);
};

} // namespace server
} // namespace libbitcoin

#endif
