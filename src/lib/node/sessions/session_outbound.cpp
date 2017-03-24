/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
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
#include <metaverse/lib/node/sessions/session_outbound.hpp>

#include <metaverse/lib/blockchain.hpp>
#include <metaverse/lib/network.hpp>
#include <metaverse/lib/node/protocols/protocol_block_in.hpp>
#include <metaverse/lib/node/protocols/protocol_block_out.hpp>
#include <metaverse/lib/node/protocols/protocol_transaction_in.hpp>
#include <metaverse/lib/node/protocols/protocol_transaction_out.hpp>
#include <metaverse/lib/node/protocols/protocol_miner.hpp>

namespace libbitcoin {
namespace node {

using namespace bc::blockchain;
using namespace bc::network;
using namespace std::placeholders;

session_outbound::session_outbound(p2p& network, block_chain& blockchain,
    transaction_pool& pool)
  : network::session_outbound(network),
    blockchain_(blockchain),
    pool_(pool)
{
    log::info(LOG_NODE)
        << "Starting outbound session.";
}

void session_outbound::attach_protocols(channel::ptr channel)
{
	log::debug(LOG_NODE) << "attach protocols";
    attach<protocol_ping>(channel)->start([channel, this](const code& ec){
    	if(ec)
		{
			return;
		}
    	attach<protocol_address>(channel)->start();
		attach<protocol_block_in>(channel, blockchain_)->set_name("session_outbound").start();
		attach<protocol_block_out>(channel, blockchain_)->start();
		attach<protocol_transaction_in>(channel, blockchain_, pool_)->start();
		attach<protocol_transaction_out>(channel, blockchain_, pool_)->start();
    });

//    attach<protocol_miner>(channel, blockchain_)->start();
}

} // namespace node
} // namespace libbitcoin
