/**
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS)
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

#include <metaverse/node/protocols/protocol_miner.hpp>

#define NAME "miner"
#define CLASS protocol_miner

namespace libbitcoin {
namespace node {

protocol_miner::protocol_miner(network::p2p& network
		, network::channel::ptr channel
		, blockchain::block_chain& blockchain
		/*, blockchain::transaction_pool& pool*/)
	:protocol_events{network, channel, NAME},
	 blockchain_{blockchain},
	 /*pool_{pool},*/
	 CONSTRUCT_TRACK(protocol_miner)
{

}

void protocol_miner::start()
{
	using namespace std::placeholders;
	protocol_events::start(BIND1(handle_stop, _1));
//	pool_.subscribe_transaction(BIND3(handle_accept_transaction, _1, _2, _3));
}

void protocol_miner::handle_stop(const code& ec)
{
	log::debug(LOG_NODE) << "protocol miner handle stop," << ec.message();
}

/*
bool protocol_miner::handle_accept_transaction(const code&, const indexes&, transaction_ptr)
{
	return true;
}
*/



}// namespace node
}// namespace libbitcoin

