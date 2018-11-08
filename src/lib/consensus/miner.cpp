/**
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
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
#include <metaverse/consensus/miner.hpp>
#include <metaverse/blockchain/block_chain.hpp>
#include <metaverse/blockchain/block_chain_impl.hpp>
#include <metaverse/blockchain/validate_block.hpp>
#include <metaverse/node/p2p_node.hpp>

#include <algorithm>
#include <functional>
#include <system_error>
#include <boost/thread.hpp>
#include <metaverse/consensus/miner/MinerAux.h>
#include <metaverse/consensus/libdevcore/BasicType.h>
#include <metaverse/consensus/witness.hpp>
#include <metaverse/bitcoin/chain/script/operation.hpp>
#include <metaverse/bitcoin/config/hash160.hpp>
#include <metaverse/bitcoin/wallet/ec_public.hpp>
#include <metaverse/bitcoin/constants.hpp>
#include <metaverse/blockchain/validate_block.hpp>
#include <metaverse/blockchain/validate_transaction.hpp>

#define LOG_HEADER "consensus"
using namespace std;

namespace libbitcoin {
namespace consensus {

static BC_CONSTEXPR unsigned int min_tx_fee = 10000;

// tuples: (priority, fee_per_kb, fee, transaction_ptr)
typedef boost::tuple<double, double, uint64_t, miner::transaction_ptr> transaction_priority;

namespace {
// fee : per kb
bool sort_by_fee_per_kb(const transaction_priority& a, const transaction_priority& b)
{
    if (a.get<1>() == b.get<1>())
        return a.get<0>() < b.get<0>();
    return a.get<1>() < b.get<1>();
};

// priority : coin age
bool sort_by_priority(const transaction_priority& a, const transaction_priority& b)
{
    if (a.get<0>() == b.get<0>())
        return a.get<1>() < b.get<1>();
    return a.get<0>() < b.get<0>();
};
} // end of anonymous namespace

miner::miner(p2p_node& node)
    : node_(node)
    , state_(state::init_)
    , new_block_number_(0)
    , new_block_limit_(0)
    , accept_block_version_(chain::block_version_pow)
    , setting_(node_.chain_impl().chain_settings())
{
    if (setting_.use_testnet_rules) {
        bc::HeaderAux::set_as_testnet();
    }
}

miner::~miner()
{
    stop();
}

bool miner::get_input_etp(const transaction& tx, const std::vector<transaction_ptr>& transactions,
                          uint64_t& total_inputs, previous_out_map_t& previous_out_map) const
{
    total_inputs = 0;
    block_chain_impl& block_chain = node_.chain_impl();
    for (auto& input : tx.inputs) {
        transaction prev_tx;
        uint64_t prev_height = 0;
        uint64_t input_value = 0;
        if (block_chain.get_transaction(prev_tx, prev_height, input.previous_output.hash)) {
            input_value = prev_tx.outputs[input.previous_output.index].value;
            previous_out_map[input.previous_output] =
                std::make_pair(prev_height, prev_tx.outputs[input.previous_output.index]);
        }
        else {
            const hash_digest& hash = input.previous_output.hash;
            const auto found = [&hash](const transaction_ptr & entry)
            {
                return entry->hash() == hash;
            };
            auto it = std::find_if(transactions.begin(), transactions.end(), found);
            if (it != transactions.end()) {
                input_value = (*it)->outputs[input.previous_output.index].value;
                previous_out_map[input.previous_output] =
                    std::make_pair(max_uint64, (*it)->outputs[input.previous_output.index]);
            }
            else {
#ifdef MVS_DEBUG
                log::debug(LOG_HEADER) << "previous transaction not ready: " << encode_hash(hash);
#endif
                return false;
            }
        }

        total_inputs += input_value;
    }

    return true;
}

bool miner::get_transaction(std::vector<transaction_ptr>& transactions,
                            previous_out_map_t& previous_out_map, tx_fee_map_t& tx_fee_map) const
{
    boost::mutex mutex;
    mutex.lock();
    auto f = [&transactions, &mutex](const error_code & code, const vector<transaction_ptr>& transactions_) -> void
    {
        transactions = transactions_;
        mutex.unlock();
    };
    node_.pool().fetch(f);

    boost::unique_lock<boost::mutex> lock(mutex);

    if (transactions.empty() == false) {
        set<hash_digest> sets;
        for (auto i = transactions.begin(); i != transactions.end(); ) {
            auto& tx = **i;
            auto hash = tx.hash();

            if (sets.count(hash)) {
                // already exist, keep unique
                i = transactions.erase(i);
                continue;
            }

            uint64_t total_input_value = 0;
            bool ready = get_input_etp(tx, transactions, total_input_value, previous_out_map);
            if (!ready) {
                // erase tx but not delete it from pool if parent tx is not ready
                i = transactions.erase(i);
                continue;
            }

            uint64_t total_output_value = tx.total_output_value();
            uint64_t fee = total_input_value - total_output_value;

            // check fees
            if (fee < min_tx_fee || !blockchain::validate_transaction::check_special_fees(setting_.use_testnet_rules, tx, fee)) {
                i = transactions.erase(i);
                // delete it from pool if not enough fee
                node_.pool().delete_tx(hash);
                continue;
            }

            if (!setting_.transaction_pool_consistency) {
                auto transaction_is_ok = true;
                // check double spending
                for (const auto& input : tx.inputs) {
                    if (node_.chain_impl().get_spends_output(input.previous_output)) {
                        i = transactions.erase(i);
                        node_.pool().delete_tx(hash);
                        transaction_is_ok = false;
                        break;
                    }
                }
                if (!transaction_is_ok) {
                    continue;
                }
            }

            tx_fee_map[hash] = fee;
            sets.insert(hash);
            ++i;
        }
    }
    return transactions.empty() == false;
}

bool miner::script_hash_signature_operations_count(size_t &count, const chain::input& input, vector<transaction_ptr>& transactions)
{
    const auto& previous_output = input.previous_output;
    transaction previous_tx;
    boost::uint64_t h;
    if (node_.chain_impl().get_transaction(previous_tx, h, previous_output.hash) == false) {
        bool found = false;
        for (auto& tx : transactions) {
            if (previous_output.hash == tx->hash()) {
                previous_tx = *tx;
                found = true;
                break;
            }
        }

        if (found == false)
            return false;
    }

    const auto& previous_tx_out = previous_tx.outputs[previous_output.index];
    return blockchain::validate_block::script_hash_signature_operations_count(count, previous_tx_out.script, input.script);
}

bool miner::script_hash_signature_operations_count(
    size_t &count, const chain::input::list& inputs, vector<transaction_ptr>& transactions)
{
    count = 0;
    for (const auto& input : inputs)
    {
        size_t c = 0;
        if (script_hash_signature_operations_count(c, input, transactions) == false)
            return false;
        count += c;
    }
    return true;
}

miner::block_ptr miner::create_genesis_block(bool is_mainnet)
{
    string text;
    if (is_mainnet) {
        //BTC height 452527 witness, sent to Satoshi:12c6DSiU4Rq3P4ZxziKxzrL5LmMBrzjrJX
        text = "6e64c2098b84b04a0d9f61a60d5bc8f5f80f37e19f3ad9c39bfe419db422b33c";
    }
    else {
        text = "2017.01.18 MVS start running testnet.";
    }

    block_ptr pblock = make_shared<block>();

    // Create coinbase tx
    transaction tx_new;
    tx_new.inputs.resize(1);
    tx_new.inputs[0].previous_output = output_point(null_hash, max_uint32);
    tx_new.inputs[0].script.operations = {{chain::opcode::raw_data, {text.begin(), text.end()}}};
    tx_new.outputs.resize(1);

    // init for testnet/mainnet
    if (!is_mainnet) {
        bc::wallet::payment_address testnet_genesis_address("tPd41bKLJGf1C5RRvaiV2mytqZB6WfM1vR");
        tx_new.outputs[0].script.operations = chain::operation::to_pay_key_hash_pattern(short_hash(testnet_genesis_address));
        pblock->header.timestamp = 1479881397;
    }
    else {
        bc::wallet::payment_address genesis_address("MGqHvbaH9wzdr6oUDFz4S1HptjoKQcjRve");
        tx_new.outputs[0].script.operations = chain::operation::to_pay_key_hash_pattern(short_hash(genesis_address));
        pblock->header.timestamp = 1486796400;
    }
    tx_new.outputs[0].value = 50000000 * coin_price();

    // Add our coinbase tx as first transaction
    pblock->transactions.push_back(tx_new);

    // Fill in header
    pblock->header.previous_block_hash = null_hash;
    pblock->header.merkle = pblock->generate_merkle_root(pblock->transactions);
    pblock->header.transaction_count = 1;
    pblock->header.version = 1;
    pblock->header.bits = 1;
    pblock->header.nonce = 0;
    pblock->header.number = 0;
    pblock->header.mixhash = 0;

    return pblock;
}

miner::transaction_ptr miner::create_coinbase_tx(
    const wallet::payment_address& pay_address, uint64_t value,
    uint64_t block_height, int lock_height, uint32_t reward_lock_time)
{
    transaction_ptr ptransaction = make_shared<message::transaction_message>();

    ptransaction->inputs.resize(1);
    ptransaction->version = transaction_version::first;
    ptransaction->inputs[0].previous_output = output_point(null_hash, max_uint32);
    script_number number(block_height);
    ptransaction->inputs[0].script.operations.push_back({ chain::opcode::special, number.data() });

    ptransaction->outputs.resize(1);
    ptransaction->outputs[0].value = value;
    ptransaction->locktime = reward_lock_time;
    if (lock_height > 0) {
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
    if (it != lock_heights.end()) {
        ret = it - lock_heights.begin();
    }
    return ret;
}

uint64_t miner::calculate_block_subsidy(uint64_t block_height, bool is_testnet)
{
    return uint64_t(3 * coin_price() * pow(0.95, block_height / bucket_size));
}

uint64_t miner::calculate_lockblock_reward(uint64_t lcok_heights, uint64_t num)
{
    int lock_heights_index = get_lock_heights_index(lcok_heights);
    if (lock_heights_index >= 0) {
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
        : dpendens(_dpendens), is_need_process(_is_need_process) { hash = make_shared<hash_digest>(_hash);}
};

miner::block_ptr miner::create_new_block(const wallet::payment_address& pay_address)
{
    block_ptr pblock;
    vector<transaction_ptr> transactions;
    map<hash_digest, transaction_dependent> transaction_dependents;
    previous_out_map_t previous_out_map;
    tx_fee_map_t tx_fee_map;
    get_transaction(transactions, previous_out_map, tx_fee_map);

    vector<transaction_priority> transaction_prioritys;
    block_chain_impl& block_chain = node_.chain_impl();

    uint64_t current_block_height = 0;
    header prev_header;
    if (!block_chain.get_last_height(current_block_height)
            || !block_chain.get_header(prev_header, current_block_height)) {
        log::warning(LOG_HEADER) << "get_last_height or get_header fail. current_block_height:" << current_block_height;
        return pblock;
    } else {
        pblock = make_shared<block>();
    }

    // Create coinbase tx
    pblock->transactions.push_back(*create_coinbase_tx(pay_address, 0, current_block_height + 1, 0, 0));

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

    uint64_t total_fee = 0;
    unsigned int block_size = 0;
    unsigned int total_tx_sig_length = blockchain::validate_block::validate_block::legacy_sigops_count(*pblock->transactions.begin());
    for (auto tx : transactions)
    {
        auto tx_hash = tx->hash();
        double priority = 0;
        for (const auto& input : tx->inputs)
        {
            auto prev_pair = previous_out_map[input.previous_output];
            uint64_t prev_height = prev_pair.first;
            const auto& prev_output = prev_pair.second;

            if (prev_height != max_uint64) {
                uint64_t input_value = prev_output.value;
                priority += (double)input_value * (current_block_height - prev_height + 1);
            }
            else {
                transaction_dependents[input.previous_output.hash].hash = make_shared<hash_digest>(tx_hash);
                transaction_dependents[tx_hash].dpendens++;
            }
        }

        uint64_t serialized_size = tx->serialized_size(0);

        // Priority is sum(valuein * age) / txsize
        priority /= serialized_size;

        // This is a more accurate fee-per-kilobyte than is used by the client code, because the
        // client code rounds up the size to the nearest 1K. That's good, because it gives an
        // incentive to create smaller transactions.
        auto tx_fee = tx_fee_map[tx_hash];
        double fee_per_kb = double(tx_fee) / (double(serialized_size) / 1000.0);
        transaction_prioritys.push_back(transaction_priority(priority, fee_per_kb, tx_fee, tx));
    }

    vector<transaction_ptr> blocked_transactions;
    auto sort_func = sort_by_fee_per_kb;
    bool is_resort = false;
    make_heap(transaction_prioritys.begin(), transaction_prioritys.end(), sort_func);

    transaction_priority *next_transaction_priority = NULL;
    uint32_t reward_lock_time = current_block_height - 1;
    while (!transaction_prioritys.empty() || next_transaction_priority)
    {
        transaction_priority temp_priority;
        if (next_transaction_priority) {
            temp_priority = *next_transaction_priority;
        } else {
            temp_priority = transaction_prioritys.front();
        }

        double priority = temp_priority.get<0>();
        double fee_per_kb = temp_priority.get<1>();
        uint64_t fee = temp_priority.get<2>();
        transaction_ptr ptx = temp_priority.get<3>();

        if (next_transaction_priority) {
            next_transaction_priority = NULL;
        }
        else {
            pop_heap(transaction_prioritys.begin(), transaction_prioritys.end(), sort_func);
            transaction_prioritys.pop_back();
        }

        hash_digest h = ptx->hash();
        if (transaction_dependents[h].dpendens != 0) {
            transaction_dependents[h].transaction = temp_priority;
            transaction_dependents[h].is_need_process = true;
            continue;
        }

        // Size limits
        uint64_t serialized_size = ptx->serialized_size(1);
        vector<transaction_ptr> coinage_reward_coinbases;
        transaction_ptr coinage_reward_coinbase;
        for (const auto& output : ptx->outputs) {
            if (chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations)) {
                int lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(output.script.operations);
                coinage_reward_coinbase = create_coinbase_tx(wallet::payment_address::extract(ptx->outputs[0].script),
                                          calculate_lockblock_reward(lock_height, output.value),
                                          current_block_height + 1, lock_height, reward_lock_time);
                unsigned int tx_sig_length = blockchain::validate_block::validate_block::legacy_sigops_count(*coinage_reward_coinbase);
                if (total_tx_sig_length + tx_sig_length >= blockchain::max_block_script_sigops) {
                    continue;
                }

                total_tx_sig_length += tx_sig_length;
                serialized_size += coinage_reward_coinbase->serialized_size(1);
                coinage_reward_coinbases.push_back(coinage_reward_coinbase);
                --reward_lock_time;
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
            sort_func = sort_by_priority;
            is_resort = true;
            make_heap(transaction_prioritys.begin(), transaction_prioritys.end(), sort_func);
        }

        size_t c;
        if (!miner::script_hash_signature_operations_count(c, ptx->inputs, transactions)
                && total_tx_sig_length + tx_sig_length + c >= blockchain::max_block_script_sigops)
            continue;
        tx_sig_length += c;

        blocked_transactions.push_back(ptx);
        for (auto& i : coinage_reward_coinbases) {
            pblock->transactions.push_back(*i);
        }

        block_size += serialized_size;
        total_tx_sig_length += tx_sig_length;
        total_fee += fee;

        if (transaction_dependents[h].hash) {
            transaction_dependent &d = transaction_dependents[*transaction_dependents[h].hash];
            if (--d.dpendens == 0 && d.is_need_process) {
                next_transaction_priority = &d.transaction;
            }
        }
    }

    for (auto i : blocked_transactions) {
        pblock->transactions.push_back(*i);
    }

    // Fill in header
    pblock->header.number = current_block_height + 1;
    pblock->header.transaction_count = pblock->transactions.size();
    pblock->header.previous_block_hash = prev_header.hash();
    pblock->header.timestamp = get_adjust_time(pblock->header.number);

    bool can_use_dpos = false;
    if (get_accept_block_version() == chain::block_version_dpos ||
        get_accept_block_version() == chain::block_version_any) {
        can_use_dpos = pblock->can_use_dpos_consensus();
        can_use_dpos &= is_witness(pay_address);
        if (!can_use_dpos && get_accept_block_version() == chain::block_version_dpos) {
            return nullptr;
        }
    }
    uint64_t block_subsidy = calculate_block_subsidy(current_block_height + 1, setting_.use_testnet_rules);
    if (can_use_dpos) {
        block_subsidy = uint64_t(1.0 * block_subsidy / block::pow_check_point_height);
    }
    pblock->transactions[0].outputs[0].value = total_fee + block_subsidy;

    pblock->header.merkle = pblock->generate_merkle_root(pblock->transactions);
    if (can_use_dpos) {
        pblock->header.version = chain::block_version_dpos;
        pblock->header.bits = prev_header.bits;
    }
    else {
        pblock->header.version = chain::block_version_pow;
        pblock->header.bits = HeaderAux::calculateDifficulty(pblock->header, prev_header);
    }
    pblock->header.nonce = 0;
    pblock->header.mixhash = 0;

    return pblock;
}

chain::block_version miner::get_accept_block_version() const
{
    return accept_block_version_;
}

void miner::set_accept_block_version(chain::block_version v)
{
    accept_block_version_ = v;
}

unsigned int miner::get_adjust_time(uint64_t height) const
{
    typedef std::chrono::system_clock wall_clock;
    const auto now = wall_clock::now();
    unsigned int t = wall_clock::to_time_t(now);

    if (height >= future_blocktime_fork_height) {
        return t;
    }
    else {
        unsigned int t_past = get_median_time_past(height);
        return max(t, t_past + 1);
    }
}

unsigned int miner::get_median_time_past(uint64_t height) const
{
    block_chain_impl& block_chain = node_.chain_impl();

    int num = min<uint64_t>(height, median_time_span);
    vector<uint64_t> times;

    for (int i = 0; i < num; i++) {
        header header;
        if (block_chain.get_header(header, height - i - 1)) {
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
    auto f = [&height, &mutex](const error_code & code, boost::uint64_t new_height) -> void
    {
        if (new_height == 0 && code.value() != 0)
            log::error(LOG_HEADER) << "store_block error: " << code.message();

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

void miner::work(const wallet::payment_address& pay_address)
{
    log::info(LOG_HEADER) << "solo miner start with address: " << pay_address.encoded();
    while (state_ != state::exit_) {
        block_ptr block = create_new_block(pay_address);
        if (block) {
            if (block->header.version == chain::block_version_dpos ||
                MinerAux::search(block->header, std::bind(&miner::is_stop_miner, this, block->header.number, block))) {
                boost::uint64_t height = store_block(block);
                if (height == 0) {
                    continue;
                }

                log::info(LOG_HEADER) << "solo miner create new block at heigth:" << height;

                ++new_block_number_;
                if ((new_block_limit_ != 0) && (new_block_number_ >= new_block_limit_)) {
                    thread_.reset();
                    stop();
                    break;
                }
            }
        }
    }
}

bool miner::is_stop_miner(uint64_t block_height, block_ptr block) const
{
    if (state_ == state::exit_) {
        return true;
    }
    auto latest_height = get_height();
    if ((latest_height > block_height) ||
        (block && latest_height >= block->header.number)) {
        return true;
    }
    // if i can use pos, then exit this loop and create new block with dpos consensus next time
    if (get_accept_block_version() == chain::block_version_any &&
        block && !block->must_use_pow_consensus()) {
        boost::mutex mutex;
        mutex.lock();
        std::vector<transaction_ptr> transactions;
        auto f = [&transactions, &mutex](const error_code& ec, const vector<transaction_ptr>& transactions_) -> void
        {
            if (error::success == ec.value()) {
                transactions = transactions_;
            }
            mutex.unlock();
        };
        node_.pool().fetch(f);

        boost::unique_lock<boost::mutex> lock(mutex);

        if (!transactions.empty()) {
            return true;
        }
    }
    return false;
}

bool miner::start(const wallet::payment_address& pay_address, uint16_t number)
{
    if (!thread_) {
        new_block_limit_ = number;
        thread_.reset(new boost::thread(bind(&miner::work, this, pay_address)));
    }
    return true;
}

bool miner::start(const std::string& public_key, uint16_t number)
{
    wallet::payment_address pay_address = libbitcoin::wallet::ec_public(public_key).to_payment_address();
    if (pay_address) {
        return start(pay_address, number);
    }
    return false;
}

bool miner::stop()
{
    if (thread_) {
        state_ = state::exit_;
        thread_->join();
        thread_.reset();
    }

    state_ = state::init_;
    new_block_number_ = 0;
    new_block_limit_ = 0;
    return true;
}

uint64_t miner::get_height() const
{
    uint64_t height = 0;
    node_.chain_impl().get_last_height(height);
    return height;
}

bool miner::set_miner_public_key(const string& public_key)
{
    libbitcoin::wallet::ec_public ec_public_key(public_key);
    pay_address_ = ec_public_key.to_payment_address();
    if (pay_address_) {
        log::debug(LOG_HEADER) << "set_miner_public_key[" << pay_address_.encoded() << "] success";
        return true;
    }
    else {
        log::error(LOG_HEADER) << "set_miner_public_key[" << public_key << "] is not availabe!";
        return false;
    }
}

bool miner::set_miner_payment_address(const bc::wallet::payment_address& address)
{
    if (address) {
        log::debug(LOG_HEADER) << "set_miner_payment_address[" << address.encoded() << "] success";
    }
    else {
        log::error(LOG_HEADER) << "set_miner_payment_address[" << address.encoded() << "] is not availabe!";
        return false;
    }

    pay_address_ = address;
    return true;
}

miner::block_ptr miner::get_block(bool is_force_create_block)
{
    static std::mutex mtx;
    std::lock_guard<std::mutex> lock(mtx);

    if (is_force_create_block) {
        new_block_ = create_new_block(pay_address_);
        log::debug(LOG_HEADER) << "force create new block";
        return new_block_;
    }

    if (!new_block_) {
        if (pay_address_) {
            new_block_ = create_new_block(pay_address_);
        } else {
            log::error(LOG_HEADER) << "get_block not set pay address";
        }
    }
    else {
        if (get_height() >= new_block_->header.number) {
            new_block_ = create_new_block(pay_address_);
        }
    }

    return new_block_;
}

bool miner::get_work(std::string& seed_hash, std::string& header_hash, std::string& boundary)
{
    block_ptr block = get_block();
    if (block) {
        header_hash = "0x" + to_string(HeaderAux::hashHead(new_block_->header));
        seed_hash = "0x" + to_string(HeaderAux::seedHash(new_block_->header));
        boundary = "0x" + to_string(HeaderAux::boundary(new_block_->header));
        return true;
    }
    return false;
}

bool miner::put_result(const std::string& nonce, const std::string& mix_hash,
                       const std::string& header_hash, const uint64_t &nounce_mask)
{
    bool ret = false;
    if (!get_block()) {
        return ret;
    }

    if (header_hash == "0x" + to_string(HeaderAux::hashHead(new_block_->header))) {
        uint64_t n_nonce = std::stoull(nonce, 0, 16);
        // nounce_mask defination is moved to the caller by chengzhiping 2018-3-15.
        uint64_t nonce_t = n_nonce ^ nounce_mask;
        new_block_->header.nonce = (u64) nonce_t;
        new_block_->header.mixhash = (FixedHash<32>::Arith)h256(mix_hash);
        uint64_t height = store_block(new_block_);
        if (height != 0) {
            log::debug(LOG_HEADER) << "put_result nonce:" << nonce << " mix_hash:"
                                   << mix_hash << " success with height:" << height;
            ret = true;
        }
        else {
            get_block(true);
            log::debug(LOG_HEADER) << "put_result nonce:" << nonce << " mix_hash:" << mix_hash << " fail";
        }
    }
    else {
        log::error(LOG_HEADER) << "put_result header_hash check fail. header_hash:"
                               << header_hash << " hashHead:" << to_string(HeaderAux::hashHead(new_block_->header));
    }

    return ret;
}

void miner::get_state(uint64_t &height, uint64_t &rate, string& difficulty, bool& is_mining)
{
    rate = MinerAux::getRate();
    block_chain_impl& block_chain = node_.chain_impl();
    header prev_header;
    block_chain.get_last_height(height);
    block_chain.get_header(prev_header, height);
    difficulty = to_string((u256)prev_header.bits);
    is_mining = thread_ ? true : false;
}

bool miner::get_block_header(chain::header& block_header, const string& para)
{
    if (para == "pending") {
        block_ptr block = get_block();
        if (block) {
            block_header = block->header;
            return true;
        }
    }
    else if (!para.empty()) {
        block_chain_impl& block_chain = node_.chain_impl();
        uint64_t height{0};
        if (para == "latest") {
            if (!block_chain.get_last_height(height)) {
                return false;
            }
        }
        else if (para == "earliest") {
            height = 0;
        }
        else if (para[0] >= '0' && para[0] <= '9') {
            height = atol(para.c_str());
        }
        else {
            return false;
        }

        if (block_chain.get_header(block_header, height)) {
            block_header.transaction_count = block_chain.get_transaction_count(height);
            return true;
        }
    }

    return false;
}

bool miner::is_witness(const wallet::payment_address& pay_address) const
{
    witness wit(node_);
    return wit.is_witness(pay_address);
}

} // consensus
} // libbitcoin
