
#include <bitcoin/consensus/miner.hpp>
#include <bitcoin/blockchain/block_chain.hpp>
#include <bitcoin/blockchain/block_chain_impl.hpp>
#include <bitcoin/blockchain/validate_block.hpp>
#include <bitcoin/node/p2p_node.hpp>

#include <algorithm>
#include <functional>
#include <system_error>
#include <boost/thread.hpp>
#include <bitcoin/consensus/miner/MinerAux.h>
#include <bitcoin/consensus/libdevcore/BasicType.h>

#define LOG_HEADER "consensus"
using namespace std;

namespace libbitcoin{
namespace consensus{
typedef boost::tuple<double, double, boost::int64_t, miner::transaction_ptr> transaction_priority;

miner::miner(p2p_node& node) : node_(node), state_(state::init_)
{
}

miner::~miner()
{
	stop();
}

bool miner::get_transaction(std::vector<transaction_ptr>& transactions)
{
	bool ret = false;
	boost::mutex mutex;
	mutex.lock();
	auto f = [&transactions, &mutex](const error_code& code, const vector<transaction_ptr>& transactions_) -> void
	{
		transactions = transactions_;
		mutex.unlock();
	};
	node_.pool().fetch(f);

	boost::unique_lock<boost::mutex> lock(mutex);
	return transactions.empty() == false;
}

boost::int64_t miner::calculate_fee(int height, boost::int64_t fees)
{ 
	boost::int64_t sub_sidy = 50 * coin;

	// Subsidy is cut in half every 210000 blocks, which will occur approximately every 4 years
	sub_sidy >>= (height / 210000);

	return sub_sidy + fees;
}

bool miner::script_hash_signature_operations_count(size_t &count, chain::input& input)
{
	bool ret = false; 
	const auto& previous_output = input.previous_output;
	transaction previous_tx;
	boost::uint64_t h;
	if(dynamic_cast<block_chain_impl&>(node_.chain()).get_transaction(previous_tx, h, input.previous_output.hash))
	{ 
		const auto& previous_tx_out = previous_tx.outputs[previous_output.index]; 
		ret = blockchain::validate_block::script_hash_signature_operations_count(count, previous_tx_out.script, input.script);
	}

	return ret;
}

bool miner::script_hash_signature_operations_count(size_t &count, chain::input::list& inputs)
{
	bool ret = true;

	count = 0;
	for(auto input : inputs)
	{
		size_t c;	
		if(script_hash_signature_operations_count(c, input))
		{
			ret = false;
			break;	
		}
		count += c;
	}
	return ret;
}

#define VALUE(a) (a < 'a' ? (a - '0') : (a - 'a' + 10))
std::string transfer_public_key(const string& key)
{
       string pub_key;
       for(auto i = key.begin(); i != key.end(); i+=2){
               unsigned char a = 0;
               a = (VALUE(*i) << 4)  + VALUE(*(i+1));
               pub_key.push_back(a);
       }

       return pub_key;
}

miner::block_ptr miner::create_genesis_block()
{
	string pub_key = transfer_public_key("04a96a414d04d73d492473117059458cbd0dc780c1974a48e7400eab1c3bc0d4db01c928b08b83a3474d4b2f0d4be468a6074e8666215afd62d68c23b9edab27ce");
	string text = "it is test";
	block_ptr pblock = make_shared<block>();

	// Create coinbase tx
	transaction tx_new;
	tx_new.inputs.resize(1);
	tx_new.inputs[0].previous_output = {null_hash, max_uint32};
	tx_new.inputs[0].script.operations = {{chain::opcode::raw_data,{text.begin(), text.end()}}};
	tx_new.outputs.resize(1);
	tx_new.outputs[0].script.operations = {{chain::opcode::special, {pub_key.begin(), pub_key.end()}}, {chain::opcode::checksig, {}}};
	//tx_new.output[0].scriptPubKey << pubkey << OP_CHECKSIG;
	tx_new.outputs[0].value = 50000000 * coin;

	// Add our coinbase tx as first transaction
	pblock->transactions.push_back(tx_new);

	// Fill in header 
	pblock->header.previous_block_hash = {}; 
	pblock->header.merkle = pblock->generate_merkle_root(pblock->transactions); 
	pblock->header.timestamp = 1479881397;
	pblock->header.transaction_count = 1; 
	pblock->header.version = version;
	pblock->header.bits = 1;
	pblock->header.nonce = 0; 
	pblock->header.number = 0;
	pblock->header.mixhash = 0;
	
	return pblock;
}

miner::block_ptr miner::create_new_block(const string& address)
{
	block_ptr pblock = make_shared<block>();

	vector<transaction_ptr> transactions;
	get_transaction(transactions);

	vector<transaction_priority> transaction_prioritys;
	block_chain_impl& block_chain = dynamic_cast<block_chain_impl&>(node_.chain());
	block_chain_impl *pblock_chain = &block_chain;

	boost::uint64_t current_block_height = 0;
	pblock_chain->get_last_height(current_block_height);

	// Create coinbase tx
	transaction tx_new; 
	tx_new.inputs.resize(1); 
	tx_new.inputs[0].previous_output = {null_hash, max_uint32}; 
	script_number number(height_ + 1); 
	tx_new.inputs[0].script.operations.push_back({ chain::opcode::special, number.data() }); 
	
	tx_new.outputs.resize(1); 
	tx_new.outputs[0].script.operations = {{chain::opcode::special, {address.begin(), address.end()}}, {chain::opcode::checksig, {}}};
	//tx_new.output[0].scriptPubKey << pubkey << OP_CHECKSIG;

	// Add our coinbase tx as first transaction 
	pblock->transactions.push_back(tx_new); 
	pblock->header.transaction_count = 1;

	// Largest block you're willing to create:
	unsigned int block_max_size = max_block_size_gen/2;
	// Limit to betweeen 1K and max_block_size-1K for sanity:
	block_max_size = max((unsigned int)1000, min((unsigned int)(max_block_size-1000), block_max_size));

	// How much of the block should be dedicated to high-priority transactions,
	// included regardless of the fees they pay
	unsigned int block_priority_size = 27000;
	block_priority_size = min(block_max_size, block_priority_size);

	// Minimum block size you want to create; block will be filled with free transactions
	// until there are no more or the block reaches this size:
	unsigned int block_min_size = 0; 
	block_min_size = min(block_max_size, block_min_size);

	// Collect memory pool transactions into the block
	boost::int64_t total_fee = 0; 
	unsigned int block_size = 0; 
	unsigned int block_tx_count = 0; 
	unsigned int total_tx_sig_length = 0; 
	for(auto tx : transactions) 
	{ 
		boost::int64_t total_inputs = 0; 
		double priority = 0;
		for(auto input : tx->inputs) 
		{ 
			transaction t;
			boost::uint64_t h;
			pblock_chain->get_transaction(t, h, input.previous_output.hash);
		
			boost::int64_t input_value = t.outputs[input.previous_output.index].value;
			total_inputs += input_value;
			priority += (double)input_value * (current_block_height - h + 1);
		}
		
		boost::int64_t serialized_size = tx->serialized_size(0);
		
		// Priority is sum(valuein * age) / txsize
		priority /= serialized_size;

		// This is a more accurate fee-per-kilobyte than is used by the client code, because the
		// client code rounds up the size to the nearest 1K. That's good, because it gives an
		// incentive to create smaller transactions.
		double fee_per_kb = double(total_inputs - tx->total_output_value()) / (double(serialized_size)/1000.0);
		transaction_prioritys.push_back(transaction_priority(priority, fee_per_kb, total_inputs - tx->total_output_value(), tx)); 
	}

	auto sort_by_priority = [](const transaction_priority& a, const transaction_priority& b) -> bool 
	{ 
		if (a.get<1>() == b.get<1>()) 
			return a.get<0>() < b.get<0>(); 
		return a.get<1>() < b.get<1>(); 
	};

	auto sort_by_fee = [](const transaction_priority& a, const transaction_priority& b) -> bool 
	{ 
		if (a.get<0>() == b.get<0>()) 
			return a.get<1>() < b.get<1>(); 
		return a.get<0>() < b.get<0>(); 
	};

	function<bool(const transaction_priority&, const transaction_priority&)> sort_func = sort_by_priority; 
	bool is_resort = false; 
	make_heap(transaction_prioritys.begin(), transaction_prioritys.end(), sort_func);

	while (!transaction_prioritys.empty()) 
	{ 
		double priority = transaction_prioritys.front().get<0>(); 
		double fee_per_kb = transaction_prioritys.front().get<1>(); 
		boost::int64_t fee = transaction_prioritys.front().get<2>(); 
		transaction& tx = *(transaction_prioritys.front().get<3>());

		pop_heap(transaction_prioritys.begin(), transaction_prioritys.end(), sort_func); 
		transaction_prioritys.pop_back();

		// Size limits
		boost::int64_t serialized_size = tx.serialized_size(); 
		if (block_size + serialized_size >= block_max_size) 
			continue;

		// Legacy limits on sigOps:
		unsigned int tx_sig_length = blockchain::validate_block::validate_block::legacy_sigops_count(tx); 
		if (total_tx_sig_length + tx_sig_length >= max_block_sigops) 
			continue;

		// Skip free transactions if we're past the minimum block size: 
		if (is_resort && (fee_per_kb < min_tx_fee) && (block_size + serialized_size >= block_min_size)) 
			break;

		// Prioritize by fee once past the priority size or we run out of high-priority
		// transactions:
		if (is_resort == false &&
			((block_size + serialized_size >= block_priority_size) || (priority < coin * 144 / 250))) 
		{ 
			sort_func = sort_by_fee; 
			is_resort = true; 
			make_heap(transaction_prioritys.begin(), transaction_prioritys.end(), sort_func); 
		}

		size_t c; 
		if(!miner::script_hash_signature_operations_count(c, tx.inputs) 
			&& total_tx_sig_length + tx_sig_length + c >= max_block_sigops) 
			continue; 
		tx_sig_length += c;

		// Added
		pblock->transactions.push_back(tx); 
		++pblock->header.transaction_count;

		block_size += serialized_size; 
		++block_tx_count; 
		total_tx_sig_length += tx_sig_length; 
		total_fee += fee; 
	}
		
	pblock->transactions[0].outputs[0].value = calculate_fee(current_block_height + 1, total_fee);
	//pblock->transactions[0].intput[0].script = CScript() << OP_0 << OP_0;

	header prev_header; 
	if(pblock_chain->get_header(prev_header, current_block_height)) 
	{
		// Fill in header
		pblock->header.previous_block_hash = prev_header.hash(); 
		pblock->header.merkle = pblock->generate_merkle_root(pblock->transactions); 
		pblock->header.timestamp = get_adjust_time(); 
		pblock->header.version = version; 
		pblock->header.number = height_ + 1;
		pblock->header.bits = HeaderAux::calculateDifficulty(pblock->header, prev_header);
		pblock->header.nonce = 1;
		pblock->header.mixhash = 1;
	} 
	else 
	{ 
		pblock.reset(); 
	} 
	return pblock; 
}

unsigned int miner::get_adjust_time()
{
	unsigned int t = time(NULL);
	unsigned int t_past = get_median_time_past();
	unsigned int t_prev = 1;
	boost::uint64_t height = 0;
	block_chain_impl& block_chain = dynamic_cast<block_chain_impl&>(node_.chain());
	block_chain.get_last_height(height);
	header header;
	if(block_chain.get_header(header, height))
	{ 
		t_prev += header.timestamp;
	}

	return max(t, max(t_past + 1, t_prev));
}

unsigned int miner::get_median_time_past()
{
	block_chain_impl& block_chain = dynamic_cast<block_chain_impl&>(node_.chain());
	boost::uint64_t height = 0;
	block_chain.get_last_height(height);

	int num = min<uint64_t>(height, median_time_span);
	vector<uint64_t> times;
	
	for(int i = 0; i < num; i++)
	{
		header header;
		if(block_chain.get_header(header, num - i))
		{ 
			times.push_back(header.timestamp); 
		}
	}

	sort(times.begin(), times.end());	
	return times.empty() ? 0 : times[times.size() / 2]; 
}

uint64_t miner::store_block(block_ptr block)
{
	uint64_t height;
	boost::mutex mutex;
	mutex.lock();
	auto f = [&height, &mutex](const error_code& code, boost::uint64_t new_height) -> void
	{
		height = new_height;
		mutex.unlock();
	};
	node_.chain().store(block, f);

	boost::unique_lock<boost::mutex> lock(mutex);
	return height;
}

void miner::work(const std::string& pay_public_key)
{ 
	string public_key = transfer_public_key(pay_public_key);

	while(state_ != state::exit_)
	{ 
		height_ = get_height(); 
		block_ptr block = create_new_block(public_key);
		if(block) 
		{ 
			if(MinerAux::search(block->header, std::bind(&miner::is_stop_miner, this))){
				boost::uint64_t height = store_block(block); 
				cout << "create new block with heigth:" << height << endl;
				log::debug(LOG_HEADER) << "create new block with heigth:" << height;
			}
		} 
	} 
}

bool miner::is_stop_miner()
{
	return state_ == state::exit_ || is_block_chain_height_changed();
}

bool miner::start(const std::string& pay_address)
{
	if(!thread_) {
		thread_.reset(new boost::thread(bind(&miner::work, this, pay_address)));	
	}
	return true;
}

bool miner::stop()
{
	if(thread_){
		state_ = state::exit_;
		thread_->join();
		thread_.reset();
	}
	return true;
}

bool miner::is_block_chain_height_changed()
{
	return get_height() > height_;
}

uint64_t miner::get_height()
{
	uint64_t height = 0;
	dynamic_cast<block_chain_impl&>(node_.chain()).get_last_height(height);
	return height;
}

}
}
