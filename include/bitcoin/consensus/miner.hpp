
#ifndef MVS_CONSENSUS_MINER_HPP
#define MVS_CONSENSUS_MINER_HPP

#include <vector>
#include <boost/thread.hpp>

#include "bitcoin/blockchain/block_chain_impl.hpp"
#include "bitcoin/blockchain/transaction_pool.hpp"
#include "bitcoin/bitcoin/chain/block.hpp"
#include "bitcoin/bitcoin/chain/input.hpp"

namespace libbitcoin{
namespace node{
	class p2p_node;
}
}

namespace libbitcoin{
namespace consensus{
	BC_CONSTEXPR unsigned int min_tx_fee = 1000;
	BC_CONSTEXPR unsigned int median_time_span = 11;
	BC_CONSTEXPR uint32_t version = 1;

	BC_CONSTEXPR boost::int64_t coin = 100000000;
	//The maximum allowed size for a serialized block, in bytes (network rule)
	BC_CONSTEXPR unsigned int max_block_size = 1000000;
	// The maximum size for mined blocks 
	BC_CONSTEXPR unsigned int max_block_size_gen = max_block_size/2;
	// The maximum size for transactions we're willing to relay/mine
	BC_CONSTEXPR unsigned int max_standard_tx_size = max_block_size_gen/5;
	// The maximum allowed number of signature check operations in a block (network rule)
	BC_CONSTEXPR unsigned int max_block_sigops = max_block_size/50;
	// The maximum number of orphan transactions kept in memory
	//BC_CONSTEXPR unsigned int MAX_ORPHAN_TRANSACTIONS = max_block_size/100;
	// The maximum number of entries in an 'inv' protocol message
	//BC_CONSTEXPR unsigned int MAX_INV_SZ = 50000;
	// The maximum size of a blk?????.dat file (since 0.8)
	//BC_CONSTEXPR unsigned int MAX_BLOCKFILE_SIZE = 0x8000000; // 128 MiB
	// The pre-allocation chunk size for blk?????.dat files (since 0.8)
	//BC_CONSTEXPR unsigned int BLOCKFILE_CHUNK_SIZE = 0x1000000; // 16 MiB
	// The pre-allocation chunk size for rev?????.dat files (since 0.8)
	//BC_CONSTEXPR unsigned int UNDOFILE_CHUNK_SIZE = 0x100000; // 1 MiB
	// Fake height value used in CCoins to signify they are only in the memory pool (since 0.8)
	//BC_CONSTEXPR unsigned int MEMPOOL_HEIGHT = 0x7FFFFFFF;
	// No amount larger than this (in satoshi) is valid 
	//BC_CONSTEXPR boost::int64_t MAX_MONEY = 21000000 * COIN;

class miner
{
public:
	typedef message::block_message block;
	typedef std::shared_ptr<message::block_message> block_ptr;
	typedef chain::header header;
	typedef chain::transaction transaction;
	typedef message::transaction_message::ptr transaction_ptr;
	typedef blockchain::block_chain_impl block_chain_impl;
	typedef blockchain::transaction_pool transaction_pool;
	typedef libbitcoin::node::p2p_node p2p_node;

	miner(p2p_node& node);
	~miner();

	enum state
	{
		init_,
		exit_
	};

	bool start(const std::string& pay_public_key);
	bool stop();
	static block_ptr create_genesis_block();
	bool script_hash_signature_operations_count(size_t &count, chain::input::list& inputs);
	bool script_hash_signature_operations_count(size_t &count, chain::input& input);

private:
	void work(const std::string& pay_address);
	static boost::int64_t calculate_fee(int height, boost::int64_t fees);
	block_ptr create_new_block(const std::string& address);
	unsigned int get_adjust_time();
	unsigned int get_median_time_past();
	bool get_transaction(std::vector<transaction_ptr>&);
	bool is_exit();
	uint64_t store_block(block_ptr block);
	uint64_t get_height();
	bool is_block_chain_height_changed();
	bool is_stop_miner();

private:
	p2p_node& node_;
	std::shared_ptr<boost::thread> thread_;
	mutable state state_;
	uint64_t height_;
};

}
}

#endif
