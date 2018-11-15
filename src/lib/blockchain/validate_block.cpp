/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
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
#include <metaverse/blockchain/validate_block.hpp>

#include <set>
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <system_error>
#include <vector>
#include <metaverse/bitcoin.hpp>
#include <metaverse/blockchain/block.hpp>
#include <metaverse/blockchain/block_chain_impl.hpp>
#include <metaverse/blockchain/validate_transaction.hpp>
#include <metaverse/consensus/miner/MinerAux.h>
#include <metaverse/consensus/libdevcore/BasicType.h>
#include <metaverse/consensus/miner.hpp>
#include <metaverse/bitcoin/chain/output.hpp>

using string = std::string;

namespace libbitcoin {
namespace blockchain {

// To improve readability.
#define RETURN_IF_STOPPED() \
if (stopped()) \
    return error::service_stopped

using namespace chain;

// The default sigops count for mutisignature scripts.
static constexpr uint32_t multisig_default_sigops = 20;

// The window by which a time stamp may exceed our current time (2 hours).
static const auto time_stamp_window = asio::seconds(2 * 60 * 60);

static const auto time_stamp_window_future_blocktime_fix = asio::seconds(24);

// The nullptr option is for backward compatibility only.
validate_block::validate_block(size_t height, const block& block, bool testnet,
                               const config::checkpoint::list& checks, stopped_callback callback)
    : testnet_(testnet),
      height_(height),
      activations_(script_context::none_enabled),
      minimum_version_(0),
      current_block_(block),
      checkpoints_(checks),
      stop_callback_(callback)
{
}

void validate_block::initialize_context()
{
    activations_ = chain::get_script_context();
}

// initialize_context must be called first (to set activations_).
bool validate_block::is_active(script_context flag) const
{
    return script::is_active(activations_, flag);
}

// validate_version must be called first (to set minimum_version_).
bool validate_block::is_valid_version() const
{
    return current_block_.header.version >= minimum_version_;
}

bool validate_block::stopped() const
{
    return stop_callback_();
}

code validate_block::check_block(blockchain::block_chain_impl& chain) const
{
    // These are checks that are independent of the blockchain
    // that can be validated before saving an orphan block.

    const auto& transactions = current_block_.transactions;

    if (transactions.empty() || current_block_.serialized_size() > max_block_size)
        return error::size_limits;

    const auto& header = current_block_.header;

    if (header.version == 2 && !is_vaild_proof_of_stake(header)){
        return error::proof_of_stake;
    }
     
    if(header.version == 1 && !is_valid_proof_of_work(header))
        return error::proof_of_work;


    RETURN_IF_STOPPED();

    //TO.FIX.CHENHAO.Reject
    if (current_block_.header.number == bc::consensus::future_blocktime_fork_height) {
        // 校验未来区块时间攻击分叉点
        bc::config::checkpoint::list blocktime_checkpoints;
        blocktime_checkpoints.push_back({
            "ed11a074ce80cbf82b5724bea0d74319dc6f180198fa1bbfb562bcbd50089e63",
            bc::consensus::future_blocktime_fork_height
        });

        const auto block_hash = header.hash();
        if (!config::checkpoint::validate(block_hash, current_block_.header.number, blocktime_checkpoints)) {
            return error::checkpoints_failed;
        }
    }

    if (current_block_.header.number >= bc::consensus::future_blocktime_fork_height) {
        // 未来区块时间攻击分叉，执行新规则检查
        if (!is_valid_time_stamp_new(header.timestamp))
            return error::futuristic_timestamp;
        // 过去区块时间检查
        chain::header prev_header = fetch_block(height_ - 1);
        if (current_block_.header.timestamp < prev_header.timestamp)
            return error::timestamp_too_early;
    }
    else {
        if (!is_valid_time_stamp(header.timestamp))
            return error::futuristic_timestamp;
    }

    RETURN_IF_STOPPED();

    code ec;
    if( (ec = check_coinbase(header.version, transactions)) != error::success){
        return ec;
    }


    std::set<string> assets;
    std::set<string> asset_certs;
    std::set<string> asset_mits;
    std::set<string> dids;
    std::set<string> didaddreses;
    code first_tx_ec = error::success;
    for (const auto& tx : transactions)
    {
        RETURN_IF_STOPPED();

        const auto validate_tx = std::make_shared<validate_transaction>(chain, tx, *this);
        auto ec = validate_tx->check_transaction();
        if (!ec) {
            ec = validate_tx->check_transaction_connect_input(header.number);
        }

        for (size_t i = 0; (!ec) && (i < tx.outputs.size()); ++i) {
            const auto& output = tx.outputs[i];
            if (output.is_asset_issue()) {
                auto r = assets.insert(output.get_asset_symbol());
                if (r.second == false) {
                    log::debug(LOG_BLOCKCHAIN)
                        << "check_block asset " + output.get_asset_symbol()
                        << " already exists in block!"
                        << " " << tx.to_string(1);
                    ec = error::asset_exist;
                    break;
                }
            }
            else if (output.is_asset_cert()) {
                auto&& key = output.get_asset_cert().get_key();
                auto r = asset_certs.insert(key);
                if (r.second == false) {
                    log::debug(LOG_BLOCKCHAIN)
                        << "check_block cert " + output.get_asset_cert_symbol()
                        << " with type " << output.get_asset_cert_type()
                        << " already exists in block!"
                        << " " << tx.to_string(1);
                    ec = error::asset_cert_exist;
                    break;
                }
            }
            else if (output.is_asset_mit()) {
                auto r = asset_mits.insert(output.get_asset_symbol());
                if (r.second == false) {
                    log::debug(LOG_BLOCKCHAIN)
                        << "check_block mit " + output.get_asset_symbol()
                        << " already exists in block!"
                        << " " << tx.to_string(1);
                    ec = error::mit_exist;
                    break;
                }
            }
            else if (output.is_did()) {
                auto didexist = dids.insert(output.get_did_symbol());
                if (didexist.second == false) {
                    log::debug(LOG_BLOCKCHAIN)
                        << "check_block did " + output.get_did_symbol()
                        << " already exists in block!"
                        << " " << tx.to_string(1);
                    ec = error::did_exist;
                    break;
                }

                auto didaddress = didaddreses.insert(output.get_did_address());
                if (didaddress.second == false ) {
                    log::debug(LOG_BLOCKCHAIN)
                        << "check_block did " + output.get_did_address()
                        << " address_registered_did!"
                        << " " << tx.to_string(1);
                    ec = error::address_registered_did;
                    break;
                }
            }
        }

        if (ec) {
            if (!first_tx_ec) {
                first_tx_ec = ec;
            }
            chain.pool().delete_tx(tx.hash());
        }
    }

    if (first_tx_ec) {
        return first_tx_ec;
    }

    RETURN_IF_STOPPED();

    if (!is_distinct_tx_set(transactions))
    {
        log::warning(LOG_BLOCKCHAIN) << "is_distinct_tx_set!!!";
        return error::duplicate;
    }

    RETURN_IF_STOPPED();

    const auto sigops = legacy_sigops_count(transactions);
    if (sigops > max_block_script_sigops)
        return error::too_many_sigs;

    RETURN_IF_STOPPED();

    if (header.merkle != block::generate_merkle_root(transactions))
        return error::merkle_mismatch;

    return error::success;
}

code validate_block::check_coinbase(const uint32_t & version, const chain::transaction::list& txs) const
{
    unsigned int coinbase_count = 0;
    unsigned int coinstake_count = 0;
    for (auto & tx : txs) {
        if (tx.is_coinbase()) {
            if (tx.outputs.size() > 1 || tx.outputs[0].is_etp() == false) {
                return error::first_not_coinbase;
            }
            ++coinbase_count;
        }
    }

    if(version == 2){
        if( txs.size() < 2 || !txs[1].is_coinstake())
        {
            return error::tx_not_coinstake;
        }
        ++coinstake_count;
    }
    
    
    if (coinbase_count == 0) {
        return error::first_not_coinbase;
    }

    for (auto it = txs.begin() + coinbase_count + coinstake_count; it != txs.end(); ++it)
    {
        RETURN_IF_STOPPED();

        if (it->is_coinbase())
            return error::extra_coinbases;
    }

    return error::success;
}


bool validate_block::is_distinct_tx_set(const transaction::list& txs)
{
    // We define distinctness by transaction hash.
    const auto hasher = [](const transaction & transaction)
    {
        return transaction.hash();
    };

    std::vector<hash_digest> hashes(txs.size());
    std::transform(txs.begin(), txs.end(), hashes.begin(), hasher);
    std::sort(hashes.begin(), hashes.end());
    auto distinct_end = std::unique(hashes.begin(), hashes.end());
#ifdef MVS_DEBUG
    if (distinct_end != hashes.end()) {
        for (auto item : txs)
            log::warning(LOG_BLOCKCHAIN) << "hash:" << encode_hash(item.hash()) << " data:" << item.to_string(1);
    }
#endif
    return distinct_end == hashes.end();
}

bool validate_block::is_valid_time_stamp(uint32_t timestamp) const
{
    return check_time_stamp(timestamp, time_stamp_window);
}

bool validate_block::is_valid_time_stamp_new(uint32_t timestamp) const
{
    return check_time_stamp(timestamp, time_stamp_window_future_blocktime_fix);
}

bool validate_block::check_time_stamp(uint32_t timestamp, const asio::seconds& window) const
{
    // Use system clock because we require accurate time of day.
    typedef std::chrono::system_clock wall_clock;
    const auto block_time = wall_clock::from_time_t(timestamp);
    const auto future_time = wall_clock::now() + window;
    return block_time <= future_time;
}

// TODO: move to bc::chain::opcode.
// Determine if code is in the op_n range.
inline bool within_op_n(opcode code)
{
    const auto value = static_cast<uint8_t>(code);
    constexpr auto op_1 = static_cast<uint8_t>(opcode::op_1);
    constexpr auto op_16 = static_cast<uint8_t>(opcode::op_16);
    return op_1 <= value && value <= op_16;
}

// TODO: move to bc::chain::opcode.
// Return the op_n index (i.e. value of n).
inline uint8_t decode_op_n(opcode code)
{
    BITCOIN_ASSERT(within_op_n(code));
    const auto value = static_cast<uint8_t>(code);
    constexpr auto op_0 = static_cast<uint8_t>(opcode::op_1) - 1;
    return value - op_0;
}

// TODO: move to bc::chain::operation::stack.
inline size_t count_script_sigops(const operation::stack& operations,
                                  bool accurate)
{
    size_t total_sigs = 0;
    opcode last_opcode = opcode::bad_operation;
    for (const auto& op : operations)
    {
        if (op.code == opcode::checksig ||
                op.code == opcode::checksigverify)
        {
            total_sigs++;
        }
        else if (op.code == opcode::checkmultisig ||
                 op.code == opcode::checkmultisigverify)
        {
            if (accurate && within_op_n(last_opcode))
                total_sigs += decode_op_n(last_opcode);
            else
                total_sigs += multisig_default_sigops;
        }

        last_opcode = op.code;
    }

    return total_sigs;
}

// TODO: move to bc::chain::transaction.
size_t validate_block::legacy_sigops_count(const transaction& tx)
{
    size_t total_sigs = 0;
    for (const auto& input : tx.inputs)
    {
        const auto& operations = input.script.operations;
        total_sigs += count_script_sigops(operations, false);
    }

    for (const auto& output : tx.outputs)
    {
        const auto& operations = output.script.operations;
        total_sigs += count_script_sigops(operations, false);
    }

    return total_sigs;
}

size_t validate_block::legacy_sigops_count(const transaction::list& txs)
{
    size_t total_sigs = 0;
    for (const auto& tx : txs)
        total_sigs += legacy_sigops_count(tx);

    return total_sigs;
}

// BUGBUG: we should confirm block hash doesn't exist.
code validate_block::accept_block() const
{
    const auto& header = current_block_.header;
    if (header.bits != work_required(testnet_))
        return error::incorrect_proof_of_work;

    RETURN_IF_STOPPED();

    //CHENHAO. future blocktime attack
#if 0
    if (header.number >= bc::consensus::future_blocktime_fork_height) {

    } else {
        if (header.timestamp <= median_time_past())
            return error::timestamp_too_early;
    }
#endif


    RETURN_IF_STOPPED();

    // Txs should be final when included in a block.
    for (const auto& tx : current_block_.transactions)
    {
        if (!tx.is_final(height_, header.timestamp))
            return error::non_final_transaction;

        RETURN_IF_STOPPED();
    }

    // Ensure that the block passes checkpoints.
    // This is both DOS protection and performance optimization for sync.
    const auto block_hash = header.hash();
    if (!config::checkpoint::validate(block_hash, height_, checkpoints_))
        return error::checkpoints_failed;

    RETURN_IF_STOPPED();

    // Reject blocks that are below the minimum version for the current height.
    if (!is_valid_version())
        return error::old_version_block;

    RETURN_IF_STOPPED();

    // Enforce rule that the coinbase starts with serialized height.
    if (is_active(script_context::bip34_enabled) &&
            !is_valid_coinbase_height(height_, current_block_))
        return error::coinbase_height_mismatch;

    if (is_active(script_context::bip112_enabled)) {
        for (const auto& tx : current_block_.transactions) {
            if (tx.is_locked(height_, median_time_past())) {
                return error::sequence_locked;
            }

            RETURN_IF_STOPPED();
        }
    }
    return error::success;
}

u256 validate_block::work_required(bool is_testnet) const
{
    chain::header prev_header = fetch_block(height_ - 1);
    return HeaderAux::calculateDifficulty(const_cast<chain::header&>(current_block_.header), prev_header);
}

bool validate_block::is_valid_coinbase_height(size_t height, const block& block)
{
    // There must be a transaction with an input.
    if (block.transactions.empty() ||
            block.transactions.front().inputs.empty())
        return false;

    // Get the serialized coinbase input script as a byte vector.
    const auto& actual_tx = block.transactions.front();
    const auto& actual_script = actual_tx.inputs.front().script;
    const auto actual = actual_script.to_data(false);

    // Create the expected script as a byte vector.
    script expected_script;
    script_number number(height);
    expected_script.operations.push_back({ opcode::special, number.data() });
    const auto expected = expected_script.to_data(false);

    // Require that the coinbase script match the expected coinbase script.
    return std::equal(expected.begin(), expected.end(), actual.begin());
}

code validate_block::connect_block(hash_digest& err_tx, blockchain::block_chain_impl& chain) const
{
    err_tx = null_hash;
    const auto& transactions = current_block_.transactions;

    // BIP30 duplicate exceptions are spent and are not indexed.
    if (is_active(script_context::bip30_enabled))
    {
        ////////////// TODO: parallelize. //////////////
        for (const auto& tx : transactions)
        {
            if (is_spent_duplicate(tx))
            {
                err_tx = tx.hash();
                return error::duplicate_or_spent;
            }

            RETURN_IF_STOPPED();
        }
    }

    uint64_t fees = 0;
    size_t total_sigops = 0;
    const auto count = transactions.size();
    uint32_t version = current_block_.header.version;
    size_t coinage_reward_coinbase_index = 1;
    size_t get_coinage_reward_tx_count = 0;

    ////////////// TODO: parallelize. //////////////
    for (size_t tx_index = 0; tx_index < count; ++tx_index)
    {
        uint64_t value_in = 0;
        const auto& tx = transactions[tx_index];

        // It appears that this is also checked in check_block().
        total_sigops += legacy_sigops_count(tx);
        if (total_sigops > max_block_script_sigops)
            return error::too_many_sigs;

        RETURN_IF_STOPPED();

        // Count sigops for coinbase tx, but no other checks.
        if (tx.is_coinbase())
            continue;

        for (auto& output : transactions[tx_index].outputs)
        {
            if (chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations)) {
                if (check_get_coinage_reward_transaction(transactions[coinage_reward_coinbase_index++], output) == false) {
                    return error::invalid_coinage_reward_coinbase;
                }
                ++get_coinage_reward_tx_count;
            }
        }

        RETURN_IF_STOPPED();

        // Consensus checks here.
        if (!validate_inputs(tx, tx_index, value_in, total_sigops))
        {
            log::debug(LOG_BLOCKCHAIN) << "validate inputs of block failed. tx hash:"
                << encode_hash(tx.hash());
            err_tx = tx.hash();
            return error::validate_inputs_failed;
        }

        RETURN_IF_STOPPED();

        if (!validate_transaction::tally_fees(chain, tx, value_in, fees))
        {
            err_tx = tx.hash();
            return error::fees_out_of_range;
        }
    }

    if (get_coinage_reward_tx_count != coinage_reward_coinbase_index - 1) {
        return error::invalid_coinage_reward_coinbase;
    }

    RETURN_IF_STOPPED();

    const auto& coinbase = transactions.front();
    const auto reward = coinbase.total_output_value();
    const auto value = consensus::miner::calculate_block_subsidy(height_, testnet_) + fees;
    return reward > value ? error::coinbase_too_large : error::success;
}

bool validate_block::is_spent_duplicate(const transaction& tx) const
{
    const auto tx_hash = tx.hash();

    // Is there a matching previous tx?
    if (!transaction_exists(tx_hash))
        return false;

    // Are all outputs spent?
    ////////////// TODO: parallelize. //////////////
    for (uint32_t output_index = 0; output_index < tx.outputs.size();
            ++output_index)
    {
        if (!is_output_spent({ tx_hash, output_index }))
            return false;
    }

    return true;
}

bool validate_block::validate_inputs(const transaction& tx,
                                     size_t index_in_parent, uint64_t& value_in, size_t& total_sigops) const
{
    BITCOIN_ASSERT(!tx.is_coinbase());

    ////////////// TODO: parallelize. //////////////
    for (size_t input_index = 0; input_index < tx.inputs.size(); ++input_index)
        if (!connect_input(index_in_parent, tx, input_index, value_in,
                           total_sigops))
        {
            log::warning(LOG_BLOCKCHAIN) << "Invalid input ["
                                         << encode_hash(tx.hash()) << ":"
                                         << input_index << "]";
            return false;
        }

    return true;
}

bool validate_block::script_hash_signature_operations_count(size_t& out_count,
        const script& output_script, const script& input_script)
{
    using namespace chain;
    constexpr auto strict = script::parse_mode::strict;

    if (input_script.operations.empty() ||
            output_script.pattern() != chain::script_pattern::pay_script_hash)
    {
        out_count = 0;
        return true;
    }

    const auto& last_data = input_script.operations.back().data;
    script eval_script;
    if (!eval_script.from_data(last_data, false, strict))
    {
        return false;
    }

    out_count = count_script_sigops(eval_script.operations, true);
    return true;
}

bool validate_block::get_transaction(const hash_digest& tx_hash,
                                     chain::transaction& prev_tx, size_t& prev_height) const
{
    return fetch_transaction(prev_tx, prev_height, tx_hash);
}

bool validate_block::connect_input(size_t index_in_parent,
                                   const transaction& current_tx, size_t input_index, uint64_t& value_in,
                                   size_t& total_sigops) const
{
    BITCOIN_ASSERT(input_index < current_tx.inputs.size());

    // Lookup previous output
    size_t previous_height;
    transaction previous_tx;
    const auto& input = current_tx.inputs[input_index];
    const auto& previous_output = input.previous_output;

    // This searches the blockchain and then the orphan pool up to and
    // including the current (orphan) block and excluding blocks above fork.
    if (!fetch_transaction(previous_tx, previous_height, previous_output.hash))
    {
        log::warning(LOG_BLOCKCHAIN)
                << "Failure fetching input transaction ["
                << encode_hash(previous_output.hash) << "]";
        return false;
    }

    const auto& previous_tx_out = previous_tx.outputs[previous_output.index];

    // Signature operations count if script_hash payment type.
    size_t count;
    if (!script_hash_signature_operations_count(count,
            previous_tx_out.script, input.script))
    {
        log::warning(LOG_BLOCKCHAIN) << "Invalid eval script.";
        return false;
    }

    total_sigops += count;
    if (total_sigops > max_block_script_sigops)
    {
        log::warning(LOG_BLOCKCHAIN) << "Total sigops exceeds block maximum.";
        return false;
    }

    // Get output amount
    const auto output_value = previous_tx_out.value;
    if (output_value > max_money())
    {
        log::warning(LOG_BLOCKCHAIN) << "Output money exceeds 21 million.";
        return false;
    }

    // Check coinbase maturity has been reached
    if (previous_tx.is_coinbase())
    {
        BITCOIN_ASSERT(previous_height <= height_);
        const auto height_difference = height_ - previous_height;
        if (height_difference < coinbase_maturity)
        {
            log::warning(LOG_BLOCKCHAIN) << "Immature coinbase spend attempt.";
            return false;
        }
    }

    if (!validate_transaction::check_consensus(previous_tx_out.script,
            current_tx, input_index, activations_))
    {
        log::warning(LOG_BLOCKCHAIN) << "Input script invalid consensus.";
        return false;
    }

    // Search for double spends.
    if (is_output_spent(previous_output, index_in_parent, input_index))
    {
        log::warning(LOG_BLOCKCHAIN) << "Double spend attempt.";
        return false;
    }

    // Increase value_in by this output's value
    value_in += output_value;
    if (value_in > max_money())
    {
        log::warning(LOG_BLOCKCHAIN) << "Input money exceeds 21 million.";
        return false;
    }

    return true;
}

#undef RETURN_IF_STOPPED

} // namespace blockchain
} // namespace libbitcoin
