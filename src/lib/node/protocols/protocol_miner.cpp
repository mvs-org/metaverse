/*
 * protocol_miner.cpp
 *
 *  Created on: Nov 10, 2016
 *      Author: jiang
 */

#include <bitcoin/node/protocols/protocol_miner.hpp>

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

