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
#include <metaverse/macros_define.hpp>

#include <algorithm>
#include <functional>
#include <system_error>
#include <chrono>
#include <ctime>
#include <metaverse/consensus/miner/MinerAux.h>
#include <metaverse/consensus/libdevcore/BasicType.h>
#include <metaverse/consensus/witness.hpp>
#include <metaverse/bitcoin/chain/script/operation.hpp>
#include <metaverse/bitcoin/config/hash160.hpp>
#include <metaverse/bitcoin/wallet/ec_public.hpp>
#include <metaverse/bitcoin/utility/random.hpp>
#include <metaverse/bitcoin/constants.hpp>
#include <metaverse/blockchain/validate_block.hpp>
#include <metaverse/blockchain/validate_transaction.hpp>
#include <metaverse/blockchain/block_chain.hpp>
#include <metaverse/blockchain/block_chain_impl.hpp>
#include <metaverse/node/p2p_node.hpp>
#include <metaverse/macros_define.hpp>

#define LOG_HEADER "Miner"

namespace libbitcoin {
namespace consensus {

static BC_CONSTEXPR uint32_t min_tx_fee     = 10000;

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

std::string timestamp_to_string(uint32_t timestamp)
{
    typedef std::chrono::system_clock wall_clock;
    auto tp = wall_clock::from_time_t(timestamp);
    std::time_t rawTime = std::chrono::system_clock::to_time_t(tp);

    char buf[sizeof("yyyy-mm-dd hh:mm:ss")] = {'\0'};
    if (std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&rawTime)) == 0)
        buf[0] = '\0'; // empty if case strftime fails
    return std::string(buf);
}

chain::operation::stack to_script_operation(const wallet::payment_address& pay_address, uint64_t lock_height = 0)
{
    if (lock_height > 0) {
        return chain::operation::to_pay_key_hash_with_lock_height_pattern(pay_address.hash(), lock_height);
    }
    else {
        return chain::operation::to_pay_key_hash_pattern(pay_address.hash());
    }
}

miner::miner(p2p_node& node)
    : node_(node)
    , state_(state::init_)
    , new_block_number_(0)
    , new_block_limit_(0)
    , accept_block_version_(chain::block_version_pow)
    , setting_(node_.chain_impl().chain_settings())
{
    if (setting_.use_testnet_rules) {
        HeaderAux::set_as_testnet();
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
    auto f = [&transactions, &mutex](const code&, const std::vector<transaction_ptr>& transactions_) -> void
    {
        transactions = transactions_;
        mutex.unlock();
    };
    node_.pool().fetch(f);

    boost::unique_lock<boost::mutex> lock(mutex);

    if (transactions.empty() == false) {
        std::set<hash_digest> sets;
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
#ifdef MVS_DEBUG
                log::debug(LOG_HEADER) << "check fees failed, delete_tx " << encode_hash(hash);
#endif
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
#ifdef MVS_DEBUG
                        log::debug(LOG_HEADER) << "check double spending failed, delete_tx " << encode_hash(hash);
#endif
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

bool miner::script_hash_signature_operations_count(uint64_t &count, const chain::input& input, std::vector<transaction_ptr>& transactions)
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
    return blockchain::validate_block::script_hash_signature_operations_count(
        count, previous_tx_out.script, input.script);
}

bool miner::script_hash_signature_operations_count(
    uint64_t &count, const chain::input::list& inputs, std::vector<transaction_ptr>& transactions)
{
    count = 0;
    for (const auto& input : inputs)
    {
        uint64_t c = 0;
        if (script_hash_signature_operations_count(c, input, transactions) == false)
            return false;
        count += c;
    }
    return true;
}

miner::block_ptr miner::create_genesis_block(bool is_mainnet)
{
    std::string text;
    if (is_mainnet) {
        //BTC height 452527 witness, sent to Satoshi:12c6DSiU4Rq3P4ZxziKxzrL5LmMBrzjrJX
        text = "6e64c2098b84b04a0d9f61a60d5bc8f5f80f37e19f3ad9c39bfe419db422b33c";
    }
    else {
        text = "2017.01.18 MVS start running testnet.";
    }

    block_ptr pblock = std::make_shared<block>();

    // Create coinbase tx
    transaction tx_new;
    tx_new.inputs.resize(1);
    tx_new.inputs[0].previous_output = output_point(null_hash, max_uint32);
    tx_new.inputs[0].script.operations = {{chain::opcode::raw_data, {text.begin(), text.end()}}};
    tx_new.outputs.resize(1);

    wallet::payment_address genesis_address(get_genesis_address(!is_mainnet));
    tx_new.outputs[0].script.operations = to_script_operation(genesis_address);
    tx_new.outputs[0].value = 50000000 * coin_price();

    if (!is_mainnet) {
        pblock->header.timestamp = 1479881397;
    }
    else {
        pblock->header.timestamp = 1486796400;
    }

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
    uint64_t block_height, int lock_height)
{
    transaction_ptr ptransaction = std::make_shared<message::transaction_message>();

    ptransaction->inputs.resize(1);
    ptransaction->version = transaction_version::first;
    ptransaction->inputs[0].previous_output = output_point(null_hash, max_uint32);
    script_number number(block_height);
    ptransaction->inputs[0].script.operations.push_back({ chain::opcode::special, number.data() });

    ptransaction->outputs.resize(1);
    ptransaction->outputs[0].value = value;
    ptransaction->outputs[0].script.operations = to_script_operation(pay_address, lock_height);

    return ptransaction;
}

std::shared_ptr<chain::output> miner::create_coinbase_mst_output(const wallet::payment_address& pay_address,
    const std::string& symbol, uint64_t value)
{
    using namespace bc::chain;
    chain::asset_transfer trans(symbol, value);
    chain::asset ass(ASSET_TRANSFERABLE_TYPE, trans);

    if (!ass.is_valid()) {
        return nullptr;
    }

    chain::attachment attach(ASSET_TYPE, 1/*version*/, ass);
    auto payment_script = chain::script{ to_script_operation(pay_address) };
    auto output = std::make_shared<chain::output>(0, payment_script, attach);
    return output;
}

int bucket_size = 500000;
std::vector<uint64_t> lock_heights = {25200, 108000, 331200, 655200, 1314000};
std::vector<uint64_t> coinage_rewards = {95890, 666666, 3200000, 8000000, 20000000};

int miner::get_lock_heights_index(uint64_t height)
{
    int ret = -1;
    auto it = find(lock_heights.begin(), lock_heights.end(), height);
    if (it != lock_heights.end()) {
        ret = it - lock_heights.begin();
    }
    return ret;
}

uint64_t miner::calculate_block_subsidy(uint64_t block_height, bool is_testnet, uint32_t version)
{
    if (version == chain::block_version_pow) {
        return calculate_block_subsidy_pow(block_height, is_testnet);
    }

    if (version == chain::block_version_pos) {
        return calculate_block_subsidy_pos(block_height, is_testnet);
    }

    if (version == chain::block_version_dpos) {
        return calculate_block_subsidy_dpos(block_height, is_testnet);
    }

    throw std::logic_error{"calculate_block_subsidy: unknown block version! " + std::to_string(version)};
}

uint64_t miner::calculate_block_subsidy_pow(uint64_t block_height, bool is_testnet)
{
    auto rate = block_height / bucket_size;
    if (block_height > pos_enabled_height) {
        rate = pos_enabled_height / bucket_size;
        auto period_left = pos_enabled_height % bucket_size;
        auto period_right = (bucket_size - period_left) * 2;
        auto period_end = pos_enabled_height + period_right;

        if (block_height >= period_end) {
            rate = rate + 1 + (block_height - period_end) / (2 * bucket_size);
        }
    }

    return uint64_t(3 * coin_price() * pow(0.95, rate));
}

uint64_t miner::calculate_block_subsidy_pos(uint64_t block_height, bool is_testnet)
{
    return coin_price(1);
}

uint64_t miner::calculate_block_subsidy_dpos(uint64_t block_height, bool is_testnet)
{
    auto result = calculate_block_subsidy_pow(block_height, is_testnet);
    result /= witness::get().get_witness_number();
    return result;
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

uint64_t miner::calculate_mst_subsidy(
    const blockchain_asset& asset, const asset_cert& cert,
    uint64_t block_height, bool is_testnet, uint32_t version)
{
    switch (version) {
    case chain::block_version_pow:
        return calculate_mst_subsidy_pow(asset, cert, block_height, is_testnet);

    case chain::block_version_pos:
        return calculate_mst_subsidy_pos(asset, cert, block_height, is_testnet);

    case chain::block_version_dpos:
        return calculate_mst_subsidy_dpos(asset, cert, block_height, is_testnet);

    default:
        throw std::logic_error{"calculate_mst_subsidy: unknown block version! " + std::to_string(version)};
    }

    return 0;
}

uint64_t miner::calculate_mst_subsidy_pow(
    const blockchain_asset& asset, const asset_cert& cert,
    uint64_t block_height, bool is_testnet)
{
    auto params = cert.get_mining_subsidy_param();
    if (nullptr == params) {
        return 0;
    }

    if (asset.get_height() >= block_height || asset.get_height() == 0) {
        return 0;
    }

    double initial_subsidy = (*params)[asset_cert::key_initial];
    double base = (*params)[asset_cert::key_base];
    uint64_t mst_bucket_size = (*params)[asset_cert::key_interval];
    auto rate = (block_height - asset.get_height()) / mst_bucket_size;
    return uint64_t(initial_subsidy * pow(base, rate));
}

uint64_t miner::calculate_mst_subsidy_pos(
    const blockchain_asset& asset, const asset_cert& cert,
    uint64_t block_height, bool is_testnet)
{
    // auto asset_detail = std::make_shared<asset_detail>(asset.get_asset());
    // if (!asset_detail) {
    //     return 0;
    // }

    // return 1 * pow(10, asset_detail->get_decimal_number());
    return calculate_mst_subsidy_pow(asset, cert, block_height, is_testnet);
}

uint64_t miner::calculate_mst_subsidy_dpos(
    const blockchain_asset& asset, const asset_cert& cert,
    uint64_t block_height, bool is_testnet)
{
    auto result = calculate_mst_subsidy_pow(asset, cert, block_height, is_testnet);
    result /= witness::get().get_witness_number();
    return result;
}

struct transaction_dependent {
    std::shared_ptr<hash_digest> hash;
    unsigned short dpendens;
    bool is_need_process;
    transaction_priority transaction;

    transaction_dependent() : dpendens(0), is_need_process(false) {}
    transaction_dependent(const hash_digest& _hash, unsigned short _dpendens, bool _is_need_process)
        : dpendens(_dpendens), is_need_process(_is_need_process) { hash = std::make_shared<hash_digest>(_hash);}
};

uint32_t miner::get_tx_sign_length(transaction_ptr tx)
{
    return blockchain::validate_block::validate_block::legacy_sigops_count(*tx);
}

bool miner::get_block_transactions(
    uint64_t last_height,
    std::vector<transaction_ptr>& txs, std::vector<transaction_ptr>& reward_txs,
    uint64_t& total_fee, uint32_t& total_tx_sig_length)
{
    block_chain_impl& block_chain = node_.chain_impl();

    uint64_t current_block_height = 0;
    header prev_header;
    if (!block_chain.get_last_height(current_block_height)
            || !block_chain.get_header(prev_header, current_block_height)) {
        log::warning(LOG_HEADER) << "get_last_height or get_header fail. current_block_height:" << current_block_height;
        return false;
    }

    std::vector<transaction_ptr> transactions;
    std::vector<transaction_priority> transaction_prioritys;
    std::map<hash_digest, transaction_dependent> transaction_dependents;
    previous_out_map_t previous_out_map;
    tx_fee_map_t tx_fee_map;

    if (!witness::is_begin_of_epoch(current_block_height + 1)) {
        get_transaction(transactions, previous_out_map, tx_fee_map);
    }

    // Largest block you're willing to create:
    uint32_t block_max_size = blockchain::max_block_size / 2;
    // Limit to betweeen 1K and max_block_size - 1K for sanity:
    block_max_size = std::max<uint32_t>(1000, std::min<uint32_t>((blockchain::max_block_size - 1000), block_max_size));

    // How much of the block should be dedicated to high-priority transactions,
    // included regardless of the fees they pay
    uint32_t block_priority_size = 27000;
    block_priority_size = std::min(block_max_size, block_priority_size);

    // Minimum block size you want to create; block will be filled with free transactions
    // until there are no more or the block reaches this size:
    uint32_t block_min_size = 0;
    block_min_size = std::min(block_max_size, block_min_size);

    uint32_t block_size = 0;
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
                priority += (double)input_value * (last_height - prev_height + 1);
            }
            else {
                transaction_dependents[input.previous_output.hash].hash = std::make_shared<hash_digest>(tx_hash);
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

    auto sort_func = sort_by_fee_per_kb;
    bool is_resort = false;
    make_heap(transaction_prioritys.begin(), transaction_prioritys.end(), sort_func);

    transaction_priority *next_transaction_priority = NULL;
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
        std::vector<transaction_ptr> coinage_reward_coinbases;
        transaction_ptr coinage_reward_coinbase;
        for (const auto& output : ptx->outputs) {
            if (chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations)) {
                int lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(output.script.operations);
                auto address = wallet::payment_address::extract(ptx->outputs[0].script);
                auto reward = calculate_lockblock_reward(lock_height, output.value);
                coinage_reward_coinbase = create_coinbase_tx(address, reward, last_height + 1, lock_height);
                uint32_t tx_sig_length = get_tx_sign_length(coinage_reward_coinbase);
                if (total_tx_sig_length + tx_sig_length >= blockchain::max_block_script_sigops) {
                    continue;
                }

                total_tx_sig_length += tx_sig_length;
                serialized_size += coinage_reward_coinbase->serialized_size(1);
                coinage_reward_coinbases.push_back(coinage_reward_coinbase);
            }
        }

        if (block_size + serialized_size >= block_max_size)
            continue;

        // Legacy limits on sigOps:
        uint32_t tx_sig_length = get_tx_sign_length(ptx);
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

        uint64_t c;
        if (!miner::script_hash_signature_operations_count(c, ptx->inputs, transactions)
                && total_tx_sig_length + tx_sig_length + c >= blockchain::max_block_script_sigops)
            continue;
        tx_sig_length += c;

        // update txs
        txs.push_back(ptx);
        for (auto i : coinage_reward_coinbases) {
            reward_txs.push_back(i);
        }

        // update fee
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

    return true;
}

miner::block_ptr miner::create_new_block(const wallet::payment_address& pay_address)
{
    block_chain_impl& block_chain = node_.chain_impl();

    // Get last block
    uint64_t last_height = 0;
    header prev_header;
    if (!block_chain.get_last_height(last_height)
            || !block_chain.get_header(prev_header, last_height)) {
        log::warning(LOG_HEADER) << "get_last_height or get_header fail. last_height:" << last_height;
        return nullptr;
    }

    if (get_accept_block_version() == chain::block_version_pow) {
        return create_new_block_pow(pay_address, prev_header);
    }

    if (get_accept_block_version() == chain::block_version_pos) {
        return create_new_block_pos(pay_address, prev_header);
    }

    if (get_accept_block_version() == chain::block_version_dpos) {
        return create_new_block_dpos(pay_address, prev_header);
    }

    throw std::logic_error{"create_new_block: unknown accept block version! " + std::to_string(get_accept_block_version())};
}

miner::block_ptr miner::create_new_block_pow(
    const wallet::payment_address& pay_address, const header& prev_header)
{
    uint64_t last_height = prev_header.number;
    uint64_t block_height = last_height + 1;
    uint32_t block_time = get_adjust_time(block_height);

    // create block
    block_ptr pblock = std::make_shared<block>();

    // Fill in header
    pblock->header.version = chain::block_version_pow;
    pblock->header.number = block_height;
    pblock->header.nonce = 0;
    pblock->header.mixhash = 0;
    pblock->header.timestamp = std::max(block_time, prev_header.timestamp);
    pblock->header.previous_block_hash = prev_header.hash();
    pblock->header.bits = get_next_target_required(pblock->header, prev_header);

    // Create coinbase tx
    transaction_ptr coinbase = create_coinbase_tx(pay_address, 0, block_height, 0);
    uint32_t total_tx_sig_length = get_tx_sign_length(coinbase);

    // Get txs
    uint64_t total_fee = 0;
    std::vector<transaction_ptr> txs;
    std::vector<transaction_ptr> reward_txs;
    get_block_transactions(last_height, txs, reward_txs, total_fee, total_tx_sig_length);

    // Update coinbase reward
    coinbase->outputs[0].value =
        total_fee + calculate_block_subsidy(block_height, setting_.use_testnet_rules, pblock->header.version);

    if (mining_asset_ != nullptr && mining_cert_ != nullptr) {
        add_coinbase_mst_output(*coinbase, pay_address, block_height, pblock->header.version);
    }

    if (witness::is_begin_of_epoch(block_height)
        && !witness::get().add_witness_vote_result(*coinbase, block_height)) {
        return nullptr;
    }

    // Put coinbase first
    pblock->transactions.push_back(*coinbase);

    // Put coinage reward_txs before txs.
    for (auto i : reward_txs) {
        pblock->transactions.push_back(*i);
    }

    // Put txs
    for (auto i : txs) {
        pblock->transactions.push_back(*i);
    }

    // Fill in header
    pblock->header.transaction_count = pblock->transactions.size();
    pblock->header.merkle = pblock->generate_merkle_root(pblock->transactions);

    return pblock;
}

miner::block_ptr miner::create_new_block_dpos(
    const wallet::payment_address& pay_address, const header& prev_header)
{
    block_chain_impl& block_chain = node_.chain_impl();
    uint64_t last_height = prev_header.number;
    uint64_t block_height = last_height + 1;
    uint32_t block_time = get_adjust_time(block_height);

    if (!prev_header.is_proof_of_work() || (block_time - prev_header.timestamp < 3)) {
        sleep_for_mseconds(1000, true);
        return nullptr;
    }

    if (is_stop_miner(block_height, nullptr)) {
        return nullptr;
    }

    if (!block_chain.can_use_dpos(block_height)) {
        return nullptr;
    }

    if (!witness::get().verify_signer(public_key_data_, block_height)) {
        // It is not my turn at current height.
        sleep_for_mseconds(500, true);
        return nullptr;
    }

    // create block
    block_ptr pblock = std::make_shared<block>();

    // Fill in header
    pblock->header.version = chain::block_version_dpos;
    pblock->header.number = block_height;
    pblock->header.nonce = 0;
    pblock->header.mixhash = 0;
    pblock->header.timestamp = std::max(block_time, prev_header.timestamp + 1);
    pblock->header.previous_block_hash = prev_header.hash();
    pblock->header.bits = prev_header.bits;

    // Create coinbase tx
    transaction_ptr coinbase = create_coinbase_tx(pay_address, 0, block_height, 0);
    uint32_t total_tx_sig_length = get_tx_sign_length(coinbase);

    // Get txs
    uint64_t total_fee = 0;
    std::vector<transaction_ptr> txs;
    std::vector<transaction_ptr> reward_txs;
    get_block_transactions(last_height, txs, reward_txs, total_fee, total_tx_sig_length);

    // Update coinbase reward
    coinbase->outputs[0].value =
        total_fee + calculate_block_subsidy(block_height, setting_.use_testnet_rules, pblock->header.version);

    if (mining_asset_ != nullptr && mining_cert_ != nullptr) {
        add_coinbase_mst_output(*coinbase, pay_address, block_height, pblock->header.version);
    }

    if (witness::is_begin_of_epoch(block_height)
        && !witness::get().add_witness_vote_result(*coinbase, block_height)) {
        return nullptr;
    }

    // Put coinbase first
    pblock->transactions.push_back(*coinbase);

    // Put coinage reward_txs before txs.
    for (auto i : reward_txs) {
        pblock->transactions.push_back(*i);
    }

    // Put txs
    for (auto i : txs) {
        pblock->transactions.push_back(*i);
    }

    // Fill in header
    pblock->header.transaction_count = pblock->transactions.size();
    pblock->header.merkle = pblock->generate_merkle_root(pblock->transactions);

    // add witness's signature to the current block header
    bc::endorsement endorse;
    if (!sign(pblock->blocksig, private_key_, pblock->header.hash())) {
        log::error(LOG_HEADER) << "witness sign failed in create_new_block";
        return nullptr;
    }

    pblock->public_key = wallet::ec_public(public_key_data_);

#ifdef ENABLE_PILLAR
    log::info(LOG_HEADER)
        << "create a DPoS block at height " << block_height
        << ", header hash is " << encode_hash(pblock->header.hash())
        << ", blocksig is " << encode_base16(pblock->blocksig)
        << ", public key is " << encode_base16(pblock->public_key);
#endif

    return pblock;
}

miner::block_ptr miner::create_new_block_pos(
    const wallet::payment_address& pay_address, const header& prev_header)
{
    block_chain_impl& block_chain = node_.chain_impl();

    uint64_t last_height = prev_header.number;
    uint64_t block_height = last_height + 1;

    // Check if PoS is eanbled at last_height
    if (block_height < pos_enabled_height) {
        log::warning(LOG_HEADER) << "PoS is not allowed at block height:" << last_height;
        return nullptr;
    }

    // Check deposited stake
    if (!block_chain.check_pos_capability(last_height, pay_address)) {
        log::error(LOG_HEADER) << "PoS mining is not allowed. no enough stake is deposited at address " << pay_address;
        sleep_for_mseconds(10 * 1000);
        return nullptr;
    }

    // check utxo stake
    chain::output_info::list stake_outputs;
    if (!block_chain.select_utxo_for_staking(last_height, pay_address, stake_outputs, 1000)) {
        log::error(LOG_HEADER) << "PoS mining is not allowed. no enough stake is holded at address " << pay_address;
        sleep_for_mseconds(10 * 1000);
        return nullptr;
    }

    // create block
    block_ptr pblock = std::make_shared<block>();

    // Fill in header
    pblock->header.version = chain::block_version_pos;  // pos
    pblock->header.number = block_height;
    pblock->header.previous_block_hash = prev_header.hash();
    pblock->header.nonce = 0;
    pblock->header.mixhash = 0;
    pblock->header.bits = get_next_target_required(pblock->header, prev_header);

    // create coinbase tx
    transaction_ptr coinbase = create_coinbase_tx(pay_address, 0, block_height, 0);
    uint32_t total_tx_sig_length = get_tx_sign_length(coinbase);

    // create coinstake
    uint32_t start_time = get_adjust_time(block_height);
    uint32_t block_time = start_time;
    transaction_ptr coinstake(nullptr);

    while (nullptr == coinstake && block_time < (start_time  + block_timespan_window / 2)) {
        pblock->header.timestamp = std::max(block_time, prev_header.timestamp + 1);
        coinstake = create_coinstake_tx(private_key_, pay_address, pblock, stake_outputs);
        if (coinstake) {
            break;
        }

        if (is_stop_miner(block_height, pblock)) {
            break;
        }

        uint32_t sleep_time = pseudo_random(200, 300);
        sleep_for_mseconds(sleep_time);
        block_time = get_adjust_time(block_height);
    }

    if (nullptr == coinstake) {
        return nullptr;
    }

    total_tx_sig_length += get_tx_sign_length(coinstake);

    // create pos genesis tx
    transaction_ptr genesis_tx(nullptr);
    if (!block_chain.pos_exist_before(block_height)) {
        genesis_tx = create_pos_genesis_tx(block_height, block_time);
        total_tx_sig_length += get_tx_sign_length(genesis_tx);
    }

    // Get txs
    uint64_t total_fee = 0;
    std::vector<transaction_ptr> txs;
    std::vector<transaction_ptr> reward_txs;
    get_block_transactions(last_height, txs, reward_txs, total_fee, total_tx_sig_length);

    // Update coinbase reward
    coinbase->outputs[0].value =
        total_fee + calculate_block_subsidy(block_height, setting_.use_testnet_rules, pblock->header.version);

    if (mining_asset_ != nullptr && mining_cert_ != nullptr) {
        add_coinbase_mst_output(*coinbase, pay_address, block_height, pblock->header.version);
    }

    if (witness::is_begin_of_epoch(block_height)
        && !witness::get().add_witness_vote_result(*coinbase, block_height)) {
        return nullptr;
    }

    // Put coinbase first
    pblock->transactions.push_back(*coinbase);

    // Put coinstake second
    pblock->transactions.push_back(*coinstake);

    // Put pos genesis tx third
    if (nullptr != genesis_tx) {
        pblock->transactions.push_back(*genesis_tx);
    }

    // Put coinage reward_txs before txs.
    for (auto i : reward_txs) {
        pblock->transactions.push_back(*i);
    }

    // Put txs
    for (auto i : txs) {
        pblock->transactions.push_back(*i);
    }

    // Fill in header
    pblock->header.transaction_count = pblock->transactions.size();
    pblock->header.merkle = pblock->generate_merkle_root(pblock->transactions);

    // Sign block
    if (!sign(pblock->blocksig, private_key_, pblock->header.hash())) {
        log::error(LOG_HEADER) << "PoS mining failed. cann't sign block.";
        return nullptr;
    }

#ifdef ENABLE_PILLAR
    log::info(LOG_HEADER)
        << "create a PoS block at height " << block_height
        << ", header hash is " << encode_hash(pblock->header.hash());
#endif
    return pblock;
}

u256 miner::get_next_target_required(const chain::header& header, const chain::header& prev_header)
{
    block_chain_impl& block_chain = node_.chain_impl();
    header::ptr last_header = block_chain.get_last_block_header(prev_header, header.version);
    header::ptr llast_header;

    if (last_header && last_header->number > 2) {
        auto height = last_header->number - 1;
        chain::header prev_last_header;
        if (block_chain.get_header(prev_last_header, height)) {
            llast_header = block_chain.get_last_block_header(prev_last_header, header.version);
        }
    }

    return HeaderAux::calculate_difficulty(header, last_header, llast_header);
}

bool miner::sign_coinstake_tx(
    const ec_secret& private_key,
    transaction_ptr coinstake)
{
    const uint8_t hash_type = chain::signature_hash_algorithm::all;

    for (uint64_t i = 0; i < coinstake->inputs.size(); ++i) {
        const chain::script& contract = coinstake->inputs[i].script;
        // gen sign
        endorsement endorse;
        if (!chain::script::create_endorsement(endorse, private_key,
                                               contract, *coinstake, i, hash_type)) {
            log::error(LOG_HEADER) << "sign_coinstake_tx: get_input_sign sign failure!";
            return false;
        }

        // do script
        wallet::ec_private ec_private_key(private_key, 0u, true);
        auto &&public_key = ec_private_key.to_public();
        data_chunk public_key_data;
        public_key.to_data(public_key_data);

        chain::script ss;
        ss.operations.push_back({chain::opcode::special, endorse});
        ss.operations.push_back({chain::opcode::special, public_key_data});

        // if pre-output script is deposit tx.
        if (contract.pattern() == chain::script_pattern::pay_key_hash_with_lock_height) {
            uint64_t lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(
                    contract.operations);
            ss.operations.push_back({chain::opcode::special, script_number(lock_height).data()});
        }

        coinstake->inputs[i].script = ss;
    }
    return true;
}

miner::transaction_ptr miner::create_pos_genesis_tx(uint64_t block_height, uint32_t block_time)
{
    auto& fmt = boost::format("MVS start POS at %1%") % timestamp_to_string(block_time);
    const std::string text(fmt.str());

    transaction_ptr genesis_tx = std::make_shared<message::transaction_message>();

    genesis_tx->inputs.resize(1);
    genesis_tx->inputs[0].previous_output = output_point(null_hash, max_uint32);
    genesis_tx->inputs[0].script.operations = {{chain::opcode::raw_data, {text.begin(), text.end()}}};

    genesis_tx->outputs.resize(1);
    wallet::payment_address pay_address(get_foundation_address(setting_.use_testnet_rules));
    genesis_tx->outputs[0].script.operations = to_script_operation(pay_address);
    genesis_tx->outputs[0].value = pos_genesis_reward;

    return genesis_tx;
}

miner::transaction_ptr miner::create_coinstake_tx(
    const ec_secret& private_key,
    const wallet::payment_address& pay_address,
    block_ptr pblock, const chain::output_info::list& stake_outputs)
{
    block_chain_impl& block_chain = node_.chain_impl();
    transaction_ptr coinstake = std::make_shared<message::transaction_message>();
    coinstake->version = transaction_version::first;
    bool enable_collect_split = setting_.collect_split_stake;

    uint64_t nCredit = 0;
    for (const auto& stake: stake_outputs) {
        if (!block_chain.check_pos_utxo_height_and_value(
            stake.height, pblock->header.number, stake.data.value)) {
            continue;
        }

        if (is_stop_miner(pblock->header.number, pblock)) {
            break;
        }

        if (MinerAux::verify_stake(pblock->header, stake)) {
            coinstake->inputs.clear();
            coinstake->outputs.clear();

            nCredit = stake.data.value;

            // generate inputs
            coinstake->inputs.emplace_back(stake.point, stake.data.script, max_input_sequence);

            // generate outputs
            coinstake->outputs.emplace_back(0, chain::script(), ATTACH_NULL_TYPE);

            break;
        }
    }

    if (coinstake->inputs.empty())
        return nullptr;

    uint64_t pos_split_limit = pos_stake_min_value * 2;
    if (enable_collect_split) {
        // Attempt to add more inputs
        for (const auto& stake: stake_outputs) {
            if (stake.data.value >= pos_stake_min_value) {
                continue;
            }

            if (nCredit >= pos_split_limit) {
                break;
            }

            if (coinstake->inputs.size() >= pos_coinstake_max_utxos) {
                break;
            }

            coinstake->inputs.emplace_back(stake.point, stake.data.script, max_input_sequence);
            nCredit += stake.data.value;
        }
    }

    auto&& script_operation = to_script_operation(pay_address);
    // auto payment_script = chain::script{script_operation};

    coinstake->outputs.emplace_back(nCredit, chain::script{script_operation},
        attachment(ETP_TYPE, 1, chain::etp(nCredit)));

    // split the output
    if (enable_collect_split && nCredit >= pos_split_limit && nCredit > pos_stake_min_value) {
        auto value = nCredit - pos_stake_min_value;
        coinstake->outputs[1].value = value;
        coinstake->outputs[1].attach_data = {ETP_TYPE, 1, chain::etp(value)};

        value = pos_stake_min_value;
        coinstake->outputs.emplace_back(value, chain::script{script_operation},
            attachment(ETP_TYPE, 1, chain::etp(value)));
    }

    // sign coinstake
    if (!sign_coinstake_tx(private_key, coinstake)) {
        return nullptr;
    }

    return coinstake;
}

chain::block_version miner::get_accept_block_version() const
{
    return accept_block_version_;
}

void miner::set_accept_block_version(chain::block_version v)
{
    accept_block_version_ = v;
}

uint32_t miner::get_adjust_time(uint64_t height) const
{
    typedef std::chrono::system_clock wall_clock;
    const auto now = wall_clock::now();
    unsigned int t = wall_clock::to_time_t(now);
    return t;
}

uint32_t miner::get_median_time_past(uint64_t height) const
{
    block_chain_impl& block_chain = node_.chain_impl();

    int num = std::min<uint64_t>(height, median_time_span);
    std::vector<uint64_t> times;

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
    auto f = [&height, &mutex](const code& ec, boost::uint64_t new_height) -> void
    {
        if (new_height == 0 && ec.value() != 0)
            log::error(LOG_HEADER) << "store_block error: " << ec.message();

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

void miner::sleep_for_mseconds(uint32_t interval, bool force)
{
    if (force || (get_accept_block_version() == chain::block_version_pos)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
    }
}

void miner::work(const wallet::payment_address& pay_address)
{
    log::info(LOG_HEADER)
        << "solo miner start with address: "
        << pay_address.encoded()
        << ", accept consensus " + chain::get_block_version(get_accept_block_version());

    while (state_ != state::exit_) {
        block_ptr block = create_new_block(pay_address);
        if (block) {
            bool can_store = (get_accept_block_version() == chain::block_version_pos)
                || block->header.version == chain::block_version_dpos
                || MinerAux::search(block->header, std::bind(&miner::is_stop_miner, this, block->header.number, block));
            if (can_store) {
                boost::uint64_t height = store_block(block);
                if (height == 0) {
                    sleep_for_mseconds(500);
                    continue;
                }

                log::info(LOG_HEADER) << "solo miner create "
                    << chain::get_block_version(block->header)
                    << " block at height: " << height
                    << ", bits: " << block->header.bits
                    << ", time: " << timestamp_to_string(block->header.timestamp);

                ++new_block_number_;
                if ((new_block_limit_ != 0) && (new_block_number_ >= new_block_limit_)) {
                    thread_.reset();
                    stop();
                    break;
                }
            }

            sleep_for_mseconds(1000);
        }
        else {
            sleep_for_mseconds(300);
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

    return false;
}

bool miner::start(const wallet::payment_address& pay_address, uint16_t number)
{
    if (get_accept_block_version() == chain::block_version_dpos) {
        if (private_key_.empty() || public_key_data_.empty()) {
            return false;
        }
    }

    if (!thread_) {
        new_block_limit_ = number;
        thread_.reset(new boost::thread(std::bind(&miner::work, this, pay_address)));
    }

    return true;
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

const wallet::payment_address& miner::get_miner_payment_address() const
{
    return pay_address_;
}

bool miner::set_miner_payment_address(const wallet::payment_address& address)
{
    log::debug(LOG_HEADER) << "set_miner_payment_address[" << address.encoded() << "] success";
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
        uint64_t height = 0;
        try {
            uint64_t n_nonce = std::stoull(nonce, 0, 16);
            // nounce_mask defination is moved to the caller
            uint64_t nonce_t = n_nonce ^ nounce_mask;
            new_block_->header.nonce = (u64) nonce_t;
            new_block_->header.mixhash = (FixedHash<32>::Arith)h256(mix_hash);
            height = store_block(new_block_);
        } catch (const std::exception& e) {
            log::debug(LOG_HEADER) << "put_result caught exception: " << e.what();
        }

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

void miner::get_state(uint64_t &height, uint64_t &rate, std::string& difficulty, bool& is_mining)
{
    rate = MinerAux::getRate();
    block_chain_impl& block_chain = node_.chain_impl();
    header prev_header;
    block_chain.get_last_height(height);
    block_chain.get_header(prev_header, height);
    difficulty = to_string((u256)prev_header.bits);
    is_mining = thread_ ? true : false;
}

bool miner::get_block_header(chain::header& block_header, const std::string& para)
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
            height = std::stoull(para);
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

bool miner::is_witness() const
{
    if (public_key_data_.empty()) {
        return false;
    }
    return witness::get().is_witness(witness::to_witness_id(public_key_data_));
}

bool miner::set_pub_and_pri_key(const std::string& pubkey, const std::string& prikey)
{
    // set private key
    if (!decode_base16(private_key_, prikey)
        || !bc::verify(private_key_) || private_key_.empty()) {
        log::error(LOG_HEADER) << "miner verify private key failed";
        return false;
    }

    // set public key, ref. signrawtx
    wallet::ec_private ec_private_key(private_key_, 0u, true);
    auto&& public_key = ec_private_key.to_public();
    public_key.to_data(public_key_data_);

    if (public_key_data_.empty()) {
        log::error(LOG_HEADER) << "miner set mining public key failed";
        return false;
    }

#ifdef ENABLE_PILLAR
    log::info(LOG_HEADER)
        << "miner set mining public key " << encode_base16(public_key_data_);
#endif

    return true;
}

std::string miner::get_mining_asset_symbol() const
{
    return mining_cert_ ? mining_cert_->get_symbol() : "";
}

bool miner::set_mining_asset_symbol(const std::string& symbol)
{
    mining_asset_ = nullptr;
    mining_cert_ = nullptr;

    auto& block_chain = const_cast<block_chain_impl&>(node_.chain_impl());
    auto asset = block_chain.get_issued_blockchain_asset(symbol);
    if (!asset) {
        log::error(LOG_HEADER) << "asset " << symbol << " does not exist!";
        return false;
    }

    auto detail = std::make_shared<asset_detail>(asset->get_asset());
    if (!detail) {
        log::error(LOG_HEADER) << "asset " << symbol << " does not exist!";
        return false;
    }

    auto cert = block_chain.get_asset_cert(symbol, asset_cert_ns::mining);
    if (!cert) {
        log::error(LOG_HEADER) << "asset " << symbol << " does not support mining!";
        return false;
    }

    if (!cert->get_mining_subsidy_param()) {
        log::error(LOG_HEADER) << "asset " << symbol << " has invalid mining subsidy parameters!";
        return false;
    }

    mining_asset_ = asset;
    mining_cert_ = cert;
    log::info(LOG_HEADER) << "set mining asset " << symbol;
    return true;
}

bool miner::add_coinbase_mst_output(chain::transaction& coinbase_tx,
    const wallet::payment_address& pay_address, uint64_t block_height, uint32_t version)
{
    if (witness::is_begin_of_epoch(block_height)) {
        return false;
    }

    if (!mining_cert_ || !mining_cert_->get_mining_subsidy_param()) {
        return false;
    }

    auto mst_value = calculate_mst_subsidy(*mining_asset_, *mining_cert_,
        block_height, setting_.use_testnet_rules, version);
    if (mst_value == 0) {
        return false;
    }

    auto symbol = mining_cert_->get_symbol();
    auto sp_mst_output = create_coinbase_mst_output(pay_address, symbol, mst_value);
    if (!sp_mst_output) {
        return false;
    }

    log::info(LOG_HEADER) << "add_coinbase_mst_output: " << symbol;
    coinbase_tx.outputs.emplace_back(*sp_mst_output);
    return true;
}

} // consensus
} // libbitcoin
