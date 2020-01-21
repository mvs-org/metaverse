/**
 * Copyright (c) 2019-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2019-2020 metaverse core developers (see MVS-AUTHORS)
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
#include <metaverse/node/sessions/session_inbound.hpp>

#include <metaverse/blockchain.hpp>
#include <metaverse/network.hpp>
#include <metaverse/node/protocols/protocol_block_in.hpp>
#include <metaverse/node/protocols/protocol_block_out.hpp>
#include <metaverse/node/protocols/protocol_transaction_in.hpp>
#include <metaverse/node/protocols/protocol_transaction_out.hpp>

namespace libbitcoin {
namespace node {

using namespace bc::blockchain;
using namespace bc::network;
using namespace std::placeholders;

session_inbound::session_inbound(p2p& network, block_chain& blockchain,
    transaction_pool& pool)
  : network::session_inbound(network),
    blockchain_(blockchain),
    pool_(pool)
{
    log::info(LOG_NODE)
        << "Starting inbound session.";
}

void session_inbound::attach_handshake_protocols(channel::ptr channel,
        result_handler handle_started)
{
    auto self = shared_from_this();
    attach<protocol_version>(channel)->start([channel, handle_started, this, self](const code& ec){
        if (!ec) {
            auto pt_ping = attach<protocol_ping>(channel);
            auto pt_address = attach<protocol_address>(channel);
            auto pt_block_in = attach<protocol_block_in>(channel, blockchain_);
            auto pt_block_out = attach<protocol_block_out>(channel, blockchain_);
            auto pt_tx_in = attach<protocol_transaction_in>(channel, blockchain_, pool_);
            auto pt_tx_out = attach<protocol_transaction_out>(channel, blockchain_, pool_);

            pt_ping->do_subscribe();
            pt_address->do_subscribe();
            pt_block_in->do_subscribe();
            pt_block_out->do_subscribe();
            pt_tx_in->do_subscribe();
            pt_tx_out->do_subscribe();

            channel->set_protocol_start_handler([pt_ping, pt_address, pt_block_in, pt_block_out, pt_tx_in, pt_tx_out]() {
                pt_ping->start();
                pt_address->start();
                pt_block_in->start();
                pt_block_out->start();
                pt_tx_in->start();
                pt_tx_out->start();
            });
        }

        if (stopped() || ec)
        {
            channel->invoke_protocol_start_handler(error::channel_stopped);
        }
        handle_started(ec);
    });

}

void session_inbound::attach_protocols(channel::ptr channel)
{
    channel->invoke_protocol_start_handler(error::success);
}

} // namespace node
} // namespace libbitcoin
