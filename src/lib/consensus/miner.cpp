
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
#include <bitcoin/bitcoin/chain/script/operation.hpp>
#include <bitcoin/bitcoin/config/hash160.hpp>
#include <bitcoin/bitcoin/wallet/ec_public.hpp>
#include <bitcoin/bitcoin/constants.hpp>
#include <bitcoin/blockchain/validate_block.hpp>

#define LOG_HEADER "consensus"
using namespace std;

namespace libbitcoin{
namespace consensus{
typedef boost::tuple<double, double, int64_t, miner::transaction_ptr> transaction_priority;

miner::miner(p2p_node& node) : node_(node), state_(state::init_), setting_(dynamic_cast<block_chain_impl&>(node_.chain()).chain_settings())
{
}

miner::~miner()
{
	stop();
}

bool miner::get_transaction(std::vector<transaction_ptr>& transactions)
{
	boost::mutex mutex;
	mutex.lock();
	auto f = [&transactions, &mutex](const error_code& code, const vector<transaction_ptr>& transactions_) -> void
	{
		transactions = transactions_;
		mutex.unlock();
	};
	node_.pool().fetch(f);

	boost::unique_lock<boost::mutex> lock(mutex);

	if(transactions.size() > 1) {
		set<hash_digest> sets;
		for(auto i = transactions.begin(); i != transactions.end(); ) {
			auto hash = (*i)->hash();
			if(sets.find(hash) == sets.end()){
				sets.insert(hash);
				++i;
			} else {
				i = transactions.erase(i);	
			}
		}
	}
	return transactions.empty() == false;
}

bool miner::script_hash_signature_operations_count(size_t &count, chain::input& input, vector<transaction_ptr>& transactions)
{
	const auto& previous_output = input.previous_output;
	transaction previous_tx;
	boost::uint64_t h;
	if(dynamic_cast<block_chain_impl&>(node_.chain()).get_transaction(previous_tx, h, input.previous_output.hash) == false){
		bool found = false;
		for(auto& tx : transactions)
		{
			if(input.previous_output.hash == tx->hash()){
				previous_tx = *tx;
				found = true;
			}
		}
		if(found == false)
			return false;
	}

	const auto& previous_tx_out = previous_tx.outputs[previous_output.index]; 
	return blockchain::validate_block::script_hash_signature_operations_count(count, previous_tx_out.script, input.script);
}

bool miner::script_hash_signature_operations_count(size_t &count, chain::input::list& inputs, vector<transaction_ptr>& transactions)
{
	count = 0;
	for(auto input : inputs)
	{
		size_t c = 0;	
		if(script_hash_signature_operations_count(c, input, transactions) == false)
			return false;
		count += c;
	}
	return true;
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

miner::block_ptr miner::create_genesis_block(bool is_mainnet)
{
	string text;
	if(is_mainnet) {
        //BTC 452527 height witness, send to Satoshi 1Ghf657Wmot55pLzQCq5Qp69CFdqsd6bhn
		text = "6e64c2098b84b04a0d9f61a60d5bc8f5f80f37e19f3ad9c39bfe419db422b33c";
	} else {
		text = "it is Test net";
	}

	block_ptr pblock = make_shared<block>();

	// Create coinbase tx
	transaction tx_new;
	tx_new.inputs.resize(1);
	tx_new.inputs[0].previous_output = {null_hash, max_uint32};
	tx_new.inputs[0].script.operations = {{chain::opcode::raw_data,{text.begin(), text.end()}}};
	tx_new.outputs.resize(1);

    // init for testnet/mainnet
    if (!is_mainnet)
    {
        libbitcoin::wallet::ec_public public_key("04a96a414d04d73d492473117059458cbd0dc780c1974a48e7400eab1c3bc0d4db01c928b08b83a3474d4b2f0d4be468a6074e8666215afd62d68c23b9edab27ce");
        tx_new.outputs[0].script.operations = chain::operation::to_pay_key_hash_pattern(short_hash(public_key.to_payment_address()));
	    pblock->header.timestamp = 1479881397;
    } else {
        bc::wallet::payment_address genesis_address("MGqHvbaH9wzdr6oUDFz4S1HptjoKQcjRve");
	    tx_new.outputs[0].script.operations = chain::operation::to_pay_key_hash_pattern(short_hash(genesis_address));
	    pblock->header.timestamp = 1486796400;
    }
	tx_new.outputs[0].value = 50000000 * coin_price();

	// Add our coinbase tx as first transaction
	pblock->transactions.push_back(tx_new);

	// Fill in header 
	pblock->header.previous_block_hash = {};
	pblock->header.merkle = pblock->generate_merkle_root(pblock->transactions); 
	pblock->header.transaction_count = 1; 
	pblock->header.version = 1;
	pblock->header.bits = 1;
	pblock->header.nonce = 0; 
	pblock->header.number = 0;
	pblock->header.mixhash = 0;
	
	return pblock;
}

miner::transaction_ptr miner::create_coinbase_tx(const wallet::payment_address& pay_address, uint64_t value, uint64_t block_height, int lock_height)
{
	transaction_ptr ptransaction = make_shared<message::transaction_message>();

	ptransaction->inputs.resize(1); 
	ptransaction->version = version;
	ptransaction->inputs[0].previous_output = {null_hash, max_uint32}; 
	script_number number(block_height); 
	ptransaction->inputs[0].script.operations.push_back({ chain::opcode::special, number.data() }); 
	
	ptransaction->outputs.resize(1);
	ptransaction->outputs[0].value = value;
	if(lock_height > 0) {
		ptransaction->outputs[0].script.operations = chain::operation::to_pay_key_hash_with_lock_height_pattern(short_hash(pay_address), lock_height);
	} else {
		ptransaction->outputs[0].script.operations = chain::operation::to_pay_key_hash_pattern(short_hash(pay_address));
	}

	return ptransaction;
}

int bucket_size = 500000;
vector<uint64_t> lock_heights = {25200, 108000, 331200, 655200, 1314000};
vector<uint64_t> coinage_rewards = {95890, 666666, 3200000, 8000000, 20000000};

int miner::get_lock_heights_index(uint64_t height)
{
	int ret = -1;
	auto it = find(lock_heights.begin(), lock_heights.end(), height);	
	if(it != lock_heights.end()) {
		ret = it - lock_heights.begin();
	}
	return ret;
}

uint64_t miner::calculate_block_subsidy(uint64_t block_height, bool is_testnet)
{
	uint64_t subsidy = 0;
	
	if(is_testnet && block_height < 22000){		//coinage reward changed compatible previous blocks
		uint64_t old_subsidys[] = {9100000000, 5000000000, 1700000000, 800000000, 400000000};
		subsidy = old_subsidys[block_height / bucket_size];
	} else {
		subsidy = uint64_t(3 * coin_price() * pow(0.95, block_height / bucket_size));
	}
	return subsidy;
}

uint64_t miner::calculate_lockblock_reward(uint64_t lcok_heights, uint64_t num)
{
	int lock_heights_index = get_lock_heights_index(lcok_heights);
	if(lock_heights_index >= 0) {
        double n = ((double )coinage_rewards[lock_heights_index]) / coin_price();
		return (uint64_t)(n * num);
	}
	return 0;
}

struct transaction_dependent {
	std::shared_ptr<hash_digest> hash;
	unsigned short dpendens;
	bool is_need_process;
	transaction_priority transaction;

	transaction_dependent() : dpendens(0), is_need_process(false) {}
	transaction_dependent(const hash_digest& _hash, unsigned short _dpendens, bool _is_need_process) 
		: dpendens(_dpendens),is_need_process(_is_need_process) { hash = make_shared<hash_digest>(_hash);}
};

miner::block_ptr miner::create_new_block(const wallet::payment_address& pay_address)
{
	block_ptr pblock;
	vector<transaction_ptr> transactions;
	map<hash_digest, transaction_dependent> transaction_dependents;
	get_transaction(transactions);

	vector<transaction_priority> transaction_prioritys;
	block_chain_impl& block_chain = dynamic_cast<block_chain_impl&>(node_.chain());

	uint64_t current_block_height = 0;
	header prev_header;
	if(!block_chain.get_last_height(current_block_height) 
		|| !block_chain.get_header(prev_header, current_block_height)) {
    	log::warning(LOG_HEADER) << "get_last_height or get_header fail. current_block_height:" << current_block_height;
		return pblock;
	} else {
		pblock = make_shared<block>();
	}

	// Create coinbase tx
	pblock->transactions.push_back(*create_coinbase_tx(pay_address, 0, current_block_height + 1, 0));

	// Largest block you're willing to create:
	unsigned int block_max_size = blockchain::max_block_size / 2;
	// Limit to betweeen 1K and max_block_size-1K for sanity:
	block_max_size = max((unsigned int)1000, min((unsigned int)(blockchain::max_block_size - 1000), block_max_size));

	// How much of the block should be dedicated to high-priority transactions,
	// included regardless of the fees they pay
	unsigned int block_priority_size = 27000;
	block_priority_size = min(block_max_size, block_priority_size);

	// Minimum block size you want to create; block will be filled with free transactions
	// until there are no more or the block reaches this size:
	unsigned int block_min_size = 0; 
	block_min_size = min(block_max_size, block_min_size);

	int64_t total_fee = 0; 
	unsigned int block_size = 0; 
	unsigned int total_tx_sig_length = blockchain::validate_block::validate_block::legacy_sigops_count(*pblock->transactions.begin());
	for(auto tx : transactions) 
	{ 
		int64_t total_inputs = 0; 
		double priority = 0;
		for(auto input : tx->inputs) 
		{ 
			transaction t;
			uint64_t h;
			int64_t input_value;
			if(block_chain.get_transaction(t, h, input.previous_output.hash)){
				input_value = t.outputs[input.previous_output.index].value;
			} else { 
				hash_digest& hash = input.previous_output.hash;
				const auto found = [&hash](const transaction_ptr& entry) 
				{ 
					return entry->hash() == hash;
				}; 
				auto it = std::find_if(transactions.begin(), transactions.end(), found);
				if(it != transactions.end()){
					input_value = (*it)->outputs[input.previous_output.index].value;
					hash_digest h = tx->hash();
					transaction_dependents[input.previous_output.hash].hash = make_shared<hash_digest>(h);
					transaction_dependents[h].dpendens++;
				} else {
					continue;
				}
			}
		
			total_inputs += input_value;
			priority += (double)input_value * (current_block_height - h + 1);
		}
		
		int64_t serialized_size = tx->serialized_size(0);
		
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

	vector<transaction_ptr> blocked_transactions;
	function<bool(const transaction_priority&, const transaction_priority&)> sort_func = sort_by_priority; 
	bool is_resort = false; 
	make_heap(transaction_prioritys.begin(), transaction_prioritys.end(), sort_func);

	transaction_priority *next_transaction_priority = NULL;
	while (!transaction_prioritys.empty() || next_transaction_priority)
	{ 
		transaction_priority temp_priority;
		if(next_transaction_priority){
			temp_priority = *next_transaction_priority;
		} else {
			temp_priority = transaction_prioritys.front();
		}

		double priority = temp_priority.get<0>(); 
		double fee_per_kb = temp_priority.get<1>(); 
		int64_t fee = temp_priority.get<2>(); 
		transaction_ptr ptx = temp_priority.get<3>();

		if(next_transaction_priority) {
			next_transaction_priority = NULL;
		} else {
			pop_heap(transaction_prioritys.begin(), transaction_prioritys.end(), sort_func); 
			transaction_prioritys.pop_back();
		}

		hash_digest h = ptx->hash();
		if(transaction_dependents[h].dpendens != 0){
			transaction_dependents[h].transaction = temp_priority;
			transaction_dependents[h].is_need_process = true;
			continue;
		}

		// Size limits
		int64_t serialized_size = ptx->serialized_size(1);
		vector<transaction_ptr> coinage_reward_coinbases;
		transaction_ptr coinage_reward_coinbase;
		for(auto& output : ptx->outputs){
			if(chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations)) {
				int lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(output.script.operations);
				coinage_reward_coinbase = create_coinbase_tx(wallet::payment_address::extract(ptx->outputs[0].script), calculate_lockblock_reward(lock_height, output.value), current_block_height + 1, lock_height);
				unsigned int tx_sig_length = blockchain::validate_block::validate_block::legacy_sigops_count(*coinage_reward_coinbase);
				if (total_tx_sig_length + tx_sig_length >= blockchain::max_block_script_sigops)
					continue;
				total_tx_sig_length += tx_sig_length;
				serialized_size += coinage_reward_coinbase->serialized_size(1);
				coinage_reward_coinbases.push_back(coinage_reward_coinbase);
			}
		}

		if (block_size + serialized_size >= block_max_size)
			continue;

		// Legacy limits on sigOps:
		unsigned int tx_sig_length = blockchain::validate_block::validate_block::legacy_sigops_count(*ptx);
		if (total_tx_sig_length + tx_sig_length >= blockchain::max_block_script_sigops)
			continue;

		// Skip free transactions if we're past the minimum block size: 
		if (is_resort && (fee_per_kb < min_tx_fee_per_kb) && (block_size + serialized_size >= block_min_size)) 
			break;

		// Prioritize by fee once past the priority size or we run out of high-priority
		// transactions:
		if (is_resort == false &&
			((block_size + serialized_size >= block_priority_size) || (priority < coin_price() * 144 / 250))) 
		{ 
			sort_func = sort_by_fee; 
			is_resort = true; 
			make_heap(transaction_prioritys.begin(), transaction_prioritys.end(), sort_func); 
		}

		size_t c;
		if(!miner::script_hash_signature_operations_count(c, ptx->inputs, transactions) 
			&& total_tx_sig_length + tx_sig_length + c >= blockchain::max_block_script_sigops) 
			continue; 
		tx_sig_length += c;

		blocked_transactions.push_back(ptx);
		for(auto& i : coinage_reward_coinbases)
		{
			pblock->transactions.push_back(*i);
		}

		block_size += serialized_size;
		total_tx_sig_length += tx_sig_length; 
		total_fee += fee;

		if(transaction_dependents[h].hash){
			transaction_dependent &d = transaction_dependents[*transaction_dependents[h].hash];
			if(--d.dpendens == 0 && d.is_need_process){
				next_transaction_priority = &d.transaction;	
			}
		}
	}

	for(auto i : blocked_transactions)
		pblock->transactions.push_back(*i);
		
	pblock->transactions[0].outputs[0].value = total_fee + calculate_block_subsidy(current_block_height + 1, setting_.use_testnet_rules);

	// Fill in header
	pblock->header.number = current_block_height + 1;
	pblock->header.transaction_count = pblock->transactions.size();
	pblock->header.previous_block_hash = prev_header.hash(); 
	pblock->header.merkle = pblock->generate_merkle_root(pblock->transactions); 
	pblock->header.timestamp = get_adjust_time(pblock->header.number);
	pblock->header.version = version; 
	pblock->header.bits = HeaderAux::calculateDifficulty(pblock->header, prev_header);
	pblock->header.nonce = 0;
	pblock->header.mixhash = 0;

	return pblock;
}

unsigned int miner::get_adjust_time(uint64_t height)
{ 
	typedef std::chrono::system_clock wall_clock;
	const auto now = wall_clock::now();
	unsigned int t = wall_clock::to_time_t(now);
	unsigned int t_past = get_median_time_past(height);

	return max(t, t_past + 1);
}

unsigned int miner::get_median_time_past(uint64_t height)
{
	block_chain_impl& block_chain = dynamic_cast<block_chain_impl&>(node_.chain());

	int num = min<uint64_t>(height, median_time_span);
	vector<uint64_t> times;
	
	for(int i = 0; i < num; i++)
	{
		header header;
		if(block_chain.get_header(header, height - i - 1))
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
		if(new_height == 0 && code.value() != 0)
			log::error(LOG_HEADER) << "store_block error_code: " << code.value();

		height = new_height;
		mutex.unlock();
	};
	node_.chain().store(block, f);

	boost::unique_lock<boost::mutex> lock(mutex);
	return height;
}

template <class _T>
std::string to_string(_T const& _t)
{ 
	std::ostringstream o; 
	o << _t; 
	return o.str();
}

void miner::work(const wallet::payment_address pay_address)
{ 
    log::info(LOG_HEADER)<<"solo miner start with address: "<<pay_address.encoded();
	while(state_ != state::exit_)
	{ 
		block_ptr block = create_new_block(pay_address);
		if(block) 
		{ 
			if(MinerAux::search(block->header, std::bind(&miner::is_stop_miner, this, block->header.number))){
				boost::uint64_t height = store_block(block); 
				log::info(LOG_HEADER) << "solo miner create new block at heigth:" << height;
			}
		} 
	} 
}

bool miner::is_stop_miner(uint64_t block_height)
{
	return state_ == state::exit_ || get_height() > block_height;
}

bool miner::start(const wallet::payment_address& pay_address)
{
	if(!thread_) {
		thread_.reset(new boost::thread(bind(&miner::work, this, pay_address)));	
	}
	return true;
}

bool miner::start(const std::string& public_key)
{
	wallet::payment_address pay_address = libbitcoin::wallet::ec_public(public_key).to_payment_address();
	if(pay_address) {
		return start(pay_address);	
	}
	return false;
}

bool miner::stop()
{
	if(thread_){
		state_ = state::exit_;
		thread_->join();
		thread_.reset();
		state_ = state::init_;
	}
	return true;
}

uint64_t miner::get_height()
{
	uint64_t height = 0;
	dynamic_cast<block_chain_impl&>(node_.chain()).get_last_height(height);
	return height;
}

bool miner::set_miner_public_key(const string& public_key)
{
	libbitcoin::wallet::ec_public ec_public_key(public_key);
	pay_address_ = ec_public_key.to_payment_address();
	if(pay_address_) {
		log::debug(LOG_HEADER) << "set_miner_public_key[" << pay_address_.encoded() << "] success";
		return true;
	} else {
		log::error(LOG_HEADER) << "set_miner_public_key[" << public_key << "] is not availabe!";
		return false;
	}
}

bool miner::set_miner_payment_address(const bc::wallet::payment_address& address)
{
	if(address) {
		log::debug(LOG_HEADER) << "set_miner_payment_address[" << address.encoded() << "] success";
	} else {
		log::error(LOG_HEADER) << "set_miner_payment_address[" << address.encoded() << "] is not availabe!";
		return false;
	}

	pay_address_ = address;
    return true;
}

miner::block_ptr miner::get_block(bool is_force_create_block)
{
	if(is_force_create_block) {
		new_block_ = create_new_block(pay_address_);
		log::debug(LOG_HEADER) << "force create new block";
		return new_block_;
	}

	if(!new_block_){
		if(pay_address_) {
			new_block_ = create_new_block(pay_address_);
		} else {
			log::error(LOG_HEADER) << "get_work not set pay address";
		}
	} else {
		if(get_height() >= new_block_->header.number){
			new_block_ = create_new_block(pay_address_);
		}
	}

	return new_block_;
}

bool miner::get_work(std::string& seed_hash, std::string& header_hash, std::string& boundary)
{
	block_ptr block = get_block();
	if(block) {
		header_hash = "0x" + to_string(HeaderAux::hashHead(new_block_->header));
		seed_hash = "0x" + to_string(HeaderAux::seedHash(new_block_->header));
		boundary = "0x" + to_string(HeaderAux::boundary(new_block_->header));
		return true; 
	}
	return false;
}

bool miner::put_result(const std::string& nonce, const std::string& mix_hash, const std::string& header_hash)
{
	bool ret = false;
	if(header_hash == "0x"+ to_string(HeaderAux::hashHead(new_block_->header))){
        auto s_nonce = "0x" + nonce;
        uint64_t n_nonce;
#ifdef MAC_OSX
        if(sscanf(s_nonce.c_str(), "%llx", &n_nonce) != 1) {
#else
        if(sscanf(s_nonce.c_str(), "%lx", &n_nonce) != 1) {
#endif
		    log::error(LOG_HEADER) << "nonce change error\n";
            return false;
        }
		uint64_t nonce_t = n_nonce ^0x6675636b6d657461;
		new_block_->header.nonce = (u64) nonce_t;
		new_block_->header.mixhash = (FixedHash<32>::Arith)h256(mix_hash);
		uint64_t height = store_block(new_block_);
		if(height != 0){
			log::debug(LOG_HEADER) << "put_result nonce:" << nonce << " mix_hash:" << mix_hash << " success with height:" << height;
			ret = true;
		} else {
			get_block(true);
			log::debug(LOG_HEADER) << "put_result nonce:" << nonce << " mix_hash:" << mix_hash << " fail";
		}
	} else {
		log::error(LOG_HEADER) << "put_result header_hash check fail. header_hash:" << header_hash << " hashHead:" << to_string(HeaderAux::hashHead(new_block_->header));
	}

	return ret;
}

void miner::get_state(uint64_t &height, uint64_t &rate, string& difficulty, bool& is_mining)
{
	rate = MinerAux::getRate();
	block_chain_impl& block_chain = dynamic_cast<block_chain_impl&>(node_.chain());
	header prev_header;
	block_chain.get_last_height(height);
	block_chain.get_header(prev_header, height);
	difficulty = to_string((u256)prev_header.bits);
	is_mining = thread_ ? true : false;
}

bool miner::get_block_header(chain::header& block_header, const string& para)
{
	if(para == "pending") {
		block_ptr block = get_block();
		if(block) {
			block_header = block->header;	
			return true;
		}
	} else if(para[0] >= '0' && para[0] <= '9'){
		block_chain_impl& block_chain = dynamic_cast<block_chain_impl&>(node_.chain());
		if(block_chain.get_header(block_header, atoi(para.c_str())))
			return true;
	}

	return false;
}

}
}
