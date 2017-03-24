/*
 * protocol_miner.hpp
 *
 *  Created on: Nov 10, 2016
 *      Author: jiang
 */

#ifndef INCLUDE_BITCOIN_NODE_PROTOCOLS_PROTOCOL_MINER_HPP_
#define INCLUDE_BITCOIN_NODE_PROTOCOLS_PROTOCOL_MINER_HPP_

#include <atomic>
#include <cstddef>
#include <memory>
#include <metaverse/lib/blockchain.hpp>
#include <metaverse/lib/network.hpp>
#include <metaverse/lib/node/define.hpp>

namespace libbitcoin
{

namespace node
{

class BCN_API protocol_miner:
		public network::protocol_events, track<protocol_miner>
{
public:
	using ptr = std::shared_ptr<protocol_miner>;
	using indexes = chain::point::indexes;
	using transaction_ptr = message::transaction_message::ptr;
	protocol_miner(network::p2p& network, network::channel::ptr channel
			, blockchain::block_chain& blockchain
			/*, blockchain::transaction_pool& pool*/);
	virtual void start();
private:
	void handle_stop(const code& ec);
//	bool handle_accept_transaction(const code&, const indexes&, transaction_ptr);
private:
	blockchain::block_chain& blockchain_;
//	blockchain::transaction_pool& pool_;
};

}// namespace node
}// namespace libbitcoin

#endif /* INCLUDE_BITCOIN_NODE_PROTOCOLS_PROTOCOL_MINER_HPP_ */
