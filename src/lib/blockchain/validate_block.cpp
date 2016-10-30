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
#include <bitcoin/blockchain/validate_block.hpp>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <system_error>
#include <vector>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/blockchain/block.hpp>
#include <bitcoin/blockchain/validate_transaction.hpp>

namespace libbitcoin {
namespace blockchain {

// To improve readability.
#define RETURN_IF_STOPPED() \
if (stopped()) \
    return error::service_stopped

using namespace chain;

// Consensus rule change activation and enforcement parameters.
static constexpr uint8_t version_4 = 4;
static constexpr uint8_t version_3 = 3;
static constexpr uint8_t version_2 = 2;
static constexpr uint8_t version_1 = 1;

// Mainnet activation parameters.
static constexpr size_t mainnet_active = 51;
static constexpr size_t mainnet_enforce = 75;
static constexpr size_t mainnet_sample = 100;

// Testnet activation parameters.
static constexpr size_t testnet_active = 750u;
static constexpr size_t testnet_enforce = 950u;
static constexpr size_t testnet_sample = 1000u;

// Block 173805 is the first mainnet block after date-based activation.
// Block 514 is the first testnet block after date-based activation.
static constexpr size_t mainnet_bip16_activation_height = 173805;
static constexpr size_t testnet_bip16_activation_height = 514;

// github.com/bitcoin/bips/blob/master/bip-0030.mediawiki#specification
static constexpr size_t mainnet_bip30_exception_height1 = 91842;
static constexpr size_t mainnet_bip30_exception_height2 = 91880;
static constexpr size_t testnet_bip30_exception_height1 = 0;
static constexpr size_t testnet_bip30_exception_height2 = 0;

// Max block size (1000000 bytes).
static constexpr uint32_t max_block_size = 1000000;

// Maximum signature operations per block (20000).
static constexpr uint32_t max_block_script_sigops = max_block_size / 50;

// The default sigops count for mutisignature scripts.
static constexpr uint32_t multisig_default_sigops = 20;

// Value used to define retargeting range constraint.
static constexpr uint64_t retargeting_factor = 4;

// Aim for blocks every 10 mins (600 seconds).
static constexpr uint64_t target_spacing_seconds = 10 * 60;

// Target readjustment every 2 weeks (1209600 seconds).
static constexpr uint64_t target_timespan_seconds = 2 * 7 * 24 * 60 * 60;

// The target number of blocks for 2 weeks of work (2016 blocks).
static constexpr uint64_t retargeting_interval = target_timespan_seconds /
    target_spacing_seconds;

// The window by which a time stamp may exceed our current time (2 hours).
static const auto time_stamp_window = asio::hours(2);

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
    const auto bip30_exception_height1 = testnet_ ?
        testnet_bip30_exception_height1 :
        mainnet_bip30_exception_height1;

    const auto bip30_exception_height2 = testnet_ ?
        testnet_bip30_exception_height2 :
        mainnet_bip30_exception_height2;

    const auto bip16_activation_height = testnet_ ?
        testnet_bip16_activation_height :
        mainnet_bip16_activation_height;

    const auto active = testnet_ ? testnet_active : mainnet_active;
    const auto enforce = testnet_ ? testnet_enforce : mainnet_enforce;
    const auto sample = testnet_ ? testnet_sample : mainnet_sample;

    // Continue even if this is too small or empty (fast and simpler).
    const auto versions = preceding_block_versions(sample);

    const auto ge_4 = [](uint8_t version) { return version >= version_4; };
    const auto ge_3 = [](uint8_t version) { return version >= version_3; };
    const auto ge_2 = [](uint8_t version) { return version >= version_2; };

    const auto count_4 = std::count_if(versions.begin(), versions.end(), ge_4);
    const auto count_3 = std::count_if(versions.begin(), versions.end(), ge_3);
    const auto count_2 = std::count_if(versions.begin(), versions.end(), ge_2);

    const auto activate = [active](size_t count) { return count >= active; };
    const auto enforced = [enforce](size_t count) { return count >= enforce; };

    // version 4/3/2 enforced based on 95% of preceding 1000 mainnet blocks.
    if (enforced(count_4))
        minimum_version_ = version_4;
    else if (enforced(count_3))
        minimum_version_ = version_3;
    else if (enforced(count_2))
        minimum_version_ = version_2;
    else
        minimum_version_ = version_1;

    // bip65 is activated based on 75% of preceding 1000 mainnet blocks.
    if (activate(count_4))
        activations_ |= script_context::bip65_enabled;

    // bip66 is activated based on 75% of preceding 1000 mainnet blocks.
    if (activate(count_3))
        activations_ |= script_context::bip66_enabled;

    // bip34 is activated based on 75% of preceding 1000 mainnet blocks.
    if (activate(count_2))
        activations_ |= script_context::bip34_enabled;

    // bip30 applies to all but two mainnet blocks that violate the rule.
    if (height_ != bip30_exception_height1 &&
        height_ != bip30_exception_height2)
        activations_ |= script_context::bip30_enabled;

    // bip16 was activated with a one-time test on mainnet/testnet (~55% rule).
    if (height_ >= bip16_activation_height)
        activations_ |= script_context::bip16_enabled;
}

// initialize_context must be called first (to set activations_).
bool validate_block::is_active(script_context flag) const
{
    if (!script::is_active(activations_, flag))
        return false;

    const auto version = current_block_.header.version;
    return
        (flag == script_context::bip65_enabled && version >= version_4) ||
        (flag == script_context::bip66_enabled && version >= version_3) ||
        (flag == script_context::bip34_enabled && version >= version_2);
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

code validate_block::check_block() const
{
    // These are checks that are independent of the blockchain
    // that can be validated before saving an orphan block.

    const auto& transactions = current_block_.transactions;

    if (transactions.empty() ||
        current_block_.serialized_size() > max_block_size)
        return error::size_limits;

    const auto& header = current_block_.header;

    if (!is_valid_proof_of_work(header.hash(), header.bits))
        return error::proof_of_work;

    RETURN_IF_STOPPED();

    if (!is_valid_time_stamp(header.timestamp))
        return error::futuristic_timestamp;

    RETURN_IF_STOPPED();

    if (!transactions.front().is_coinbase())
        return error::first_not_coinbase;

    for (auto it = ++transactions.begin(); it != transactions.end(); ++it)
    {
        RETURN_IF_STOPPED();

        if (it->is_coinbase())
            return error::extra_coinbases;
    }

    for (const auto& tx: transactions)
    {
        RETURN_IF_STOPPED();

        const auto ec = validate_transaction::check_transaction(tx);
        if (ec)
            return ec;
    }

    RETURN_IF_STOPPED();

    if (!is_distinct_tx_set(transactions))
        return error::duplicate;

    RETURN_IF_STOPPED();

    const auto sigops = legacy_sigops_count(transactions);
    if (sigops > max_block_script_sigops)
        return error::too_many_sigs;

    RETURN_IF_STOPPED();

    if (header.merkle != block::generate_merkle_root(transactions))
        return error::merkle_mismatch;

    return error::success;
}

bool validate_block::is_distinct_tx_set(const transaction::list& txs)
{
    // We define distinctness by transaction hash.
    const auto hasher = [](const transaction& transaction)
    {
        return transaction.hash();
    };

    std::vector<hash_digest> hashes(txs.size());
    std::transform(txs.begin(), txs.end(), hashes.begin(), hasher);
    std::sort(hashes.begin(), hashes.end());
    auto distinct_end = std::unique(hashes.begin(), hashes.end());
    return distinct_end == hashes.end();
}

bool validate_block::is_valid_time_stamp(uint32_t timestamp) const
{
    // Use system clock because we require accurate time of day.
    typedef std::chrono::system_clock wall_clock;
    const auto block_time = wall_clock::from_time_t(timestamp);
    const auto two_hour_future = wall_clock::now() + time_stamp_window;
    return block_time <= two_hour_future;
}

bool validate_block::is_valid_proof_of_work(hash_digest hash, uint32_t bits)
{
    hash_number target_value;
    if (!target_value.set_compact(bits) || target_value > max_target())
        return false;

    hash_number our_value;
    our_value.set_hash(hash);
    return (our_value <= target_value);
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
    for (const auto& op: operations)
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
    for (const auto& input: tx.inputs)
    {
        const auto& operations = input.script.operations;
        total_sigs += count_script_sigops(operations, false);
    }

    for (const auto& output: tx.outputs)
    {
        const auto& operations = output.script.operations;
        total_sigs += count_script_sigops(operations, false);
    }

    return total_sigs;
}

size_t validate_block::legacy_sigops_count(const transaction::list& txs)
{
    size_t total_sigs = 0;
    for (const auto& tx: txs)
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

    if (header.timestamp <= median_time_past())
        return error::timestamp_too_early;

    RETURN_IF_STOPPED();

    // Txs should be final when included in a block.
    for (const auto& tx: current_block_.transactions)
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

    return error::success;
}

uint32_t validate_block::work_required(bool is_testnet) const
{
    if (height_ == 0)
        return max_work_bits;

    const auto is_retarget_height = [](size_t height)
    {
        return height % retargeting_interval == 0;
    };

    if (is_retarget_height(height_))
    {
        // This is the total time it took for the last 2016 blocks.
        const auto actual = actual_time_span(retargeting_interval);

        // Now constrain the time between an upper and lower bound.
        const auto constrained = range_constrain(actual,
            target_timespan_seconds / retargeting_factor,
            target_timespan_seconds * retargeting_factor);

        hash_number retarget;
        retarget.set_compact(previous_block_bits());
        retarget *= constrained;
        retarget /= target_timespan_seconds;
        if (retarget > max_target())
            retarget = max_target();

        return retarget.compact();
    }

    if (!is_testnet)
        return previous_block_bits();

    // This section is testnet in not-retargeting scenario.
    // ------------------------------------------------------------------------

    const auto max_time_gap = fetch_block(height_ - 1).timestamp + 2 * 
        target_spacing_seconds;

    if (current_block_.header.timestamp > max_time_gap)
        return max_work_bits;

    const auto last_non_special_bits = [this, is_retarget_height]()
    {
        header previous_block;
        auto previous_height = height_;

        // TODO: this is very suboptimal, cache the set of change points.
        // Loop backwards until we find a difficulty change point,
        // or we find a block which does not have max_bits (is not special).
        while (!is_retarget_height(previous_height))
        {
            --previous_height;

            // Test for non-special block.
            previous_block = fetch_block(previous_height);
            if (previous_block.bits != max_work_bits)
                break;
        }

        return previous_block.bits;
    };

    return last_non_special_bits();

    // ------------------------------------------------------------------------
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

code validate_block::connect_block() const
{
    const auto& transactions = current_block_.transactions;

    // BIP30 duplicate exceptions are spent and are not indexed.
    if (is_active(script_context::bip30_enabled))
    {
        ////////////// TODO: parallelize. //////////////
        for (const auto& tx: transactions)
        {
            if (is_spent_duplicate(tx))
                return error::duplicate_or_spent;

            RETURN_IF_STOPPED();
        }
    }

    uint64_t fees = 0;
    size_t total_sigops = 0;
    const auto count = transactions.size();

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

        RETURN_IF_STOPPED();

        // Consensus checks here.
        if (!validate_inputs(tx, tx_index, value_in, total_sigops))
            return error::validate_inputs_failed;

        RETURN_IF_STOPPED();

        if (!validate_transaction::tally_fees(tx, value_in, fees))
            return error::fees_out_of_range;
    }

    RETURN_IF_STOPPED();

    const auto& coinbase = transactions.front();
    const auto reward = coinbase.total_output_value();
    const auto value = block_subsidy(height_) + fees;
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

bool script_hash_signature_operations_count(size_t& out_count,
    const script& output_script, const script& input_script)
{
    using namespace chain;
    constexpr auto strict = script::parse_mode::strict;

    if (input_script.operations.empty() ||
        output_script.pattern() != script_pattern::pay_script_hash)
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
