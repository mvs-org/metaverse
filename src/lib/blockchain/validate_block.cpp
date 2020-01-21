/**
 * Copyright (c) 2019-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2019-2020 metaverse core developers (see MVS-AUTHORS)
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
#include <metaverse/macros_define.hpp>

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
#include <metaverse/consensus/witness.hpp>

using string = std::string;

namespace libbitcoin {
namespace blockchain {

// To improve readability.
#define RETURN_IF_STOPPED() \
if (stopped()) \
    return error::service_stopped

using namespace chain;

// The window by which a time stamp may exceed our current time (2 hours).
static const auto time_stamp_window = asio::seconds(2 * 60 * 60);

static const auto time_stamp_window_future_blocktime_fix = asio::seconds(24);

// The nullptr option is for backward compatibility only.
validate_block::validate_block(uint64_t height, const block& block, bool testnet,
                               const config::checkpoint::list& checks, stopped_callback callback)
    : testnet_(testnet),
      height_(height),
      activations_(script_context::none_enabled),
      current_block_(block),
      checkpoints_(checks),
      stop_callback_(callback)
{
    initialize_context();
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
    return current_block_.header.version >= chain::block_version_min &&
        current_block_.header.version < chain::block_version_max;
}

bool validate_block::stopped() const
{
    return stop_callback_();
}

code validate_block::check_coinbase(const chain::header& prev_header, bool check_genesis_tx) const
{
    using namespace consensus;
    const auto& header = current_block_.header;
    const auto& transactions = current_block_.transactions;

    const auto is_block_version_dpos = header.is_proof_of_dpos();
    const auto is_begin_of_epoch = witness::is_begin_of_epoch(height_);

    uint64_t coinbase_count = 0, coinstake_count = 0;
    for (uint64_t index = 0; index < transactions.size(); ++index) {
        RETURN_IF_STOPPED();

        auto& tx = transactions[index];

        if (tx.is_coinstake()) {
            if (!header.is_proof_of_stake() || index != 1) {
                log::error(LOG_BLOCKCHAIN) << "Illegal coinstake in block "
                    << encode_hash(header.hash()) << ", with "
                    << transactions.size() << " txs, current tx: " << tx.to_string(1);
                return error::illegal_coinstake;
            }

            if (coinstake_count > 1) {
                log::error(LOG_BLOCKCHAIN) << "Extra coinstake in block "
                    << encode_hash(header.hash()) << ", with "
                    << transactions.size() << " txs, current tx: " << tx.to_string(1);
                return error::extra_coinstakes;
            }

            ++coinstake_count;
            continue;
        }

        if (!tx.is_coinbase()) {
            break;
        }

        if (tx.outputs.empty() || !tx.outputs[0].is_etp()) {
            return error::first_not_coinbase;
        }

        const auto is_pos_genesis = check_genesis_tx && index == 2;

        if (index == 0) {
            if (is_begin_of_epoch) {
                // <first:  coinbase reward>     (required)
                // [last:   witness vote result] (required)
                if (tx.outputs.size() != 2) {
                    return error::illegal_coinstake;
                }

                auto& vote_result_output = tx.outputs.back();
                if (!consensus::witness::is_vote_result_output(vote_result_output)) {
                    return error::illegal_coinstake;
                }
            }
            else {
                // <first:  coinbase reward>     (required)
                // [second: coinbase mst reward] (optional)
                if (tx.outputs.size() > 2 ||
                    (tx.outputs.size() == 2 && !tx.outputs[1].is_asset_transfer())) {
                    return error::illegal_coinstake;
                }
            }
        }
        else if (is_pos_genesis) {
            if (!tx.is_pos_genesis_tx(testnet_)) {
                return error::check_pos_genesis_error;
            }
        }
        else {
            if (tx.outputs.size() != 1) {
                return error::illegal_coinstake;
            }
        }

        if (is_active(script_context::bip34_enabled)) {
            // Enforce rule that the coinbase starts with serialized height.
            if (!is_pos_genesis &&
                !is_valid_coinbase_height(height_, current_block_, index)) {
                return error::coinbase_height_mismatch;
            }
        }
        else {
            const auto& coinbase_script = tx.inputs[0].script;
            const auto coinbase_size = coinbase_script.serialized_size(false);
            constexpr uint32_t max_coinbase_size = 100;
            if ((coinbase_size < 2) ||
                (coinbase_size > max_coinbase_size)) {
                return error::invalid_coinbase_script_size;
            }
        }

        ++coinbase_count;
    }

    if (coinbase_count == 0) {
        return error::first_not_coinbase;
    }

    if (header.is_proof_of_stake() && coinstake_count == 0) {
        return error::miss_coinstake;
    }

    auto iter_start = transactions.begin() + coinbase_count + coinstake_count;
    for (auto it = iter_start; it != transactions.end(); ++it) {
        RETURN_IF_STOPPED();

        if (it->is_coinbase() || it->is_coinstake()) {
            log::error(LOG_BLOCKCHAIN) << "Illegal coinbase or coinstake in block: "
                << encode_hash(header.hash()) << ", with " << transactions.size() << " txs.";
            return error::extra_coinbases;
        }
    }

    if (!consensus::witness::is_dpos_enabled()) {
        return error::success;
    }

    consensus::witness_with_validate_block_context context(witness::get(), this);

    if (is_begin_of_epoch) {
        RETURN_IF_STOPPED();
        if (!witness::get().update_witness_list(current_block_)) {
            log::error(LOG_BLOCKCHAIN)
                << "update witness list failed at " << height_
                << " block hash: " << encode_hash(header.hash());
            return error::witness_update_error;
        }
    }

    if (is_block_version_dpos) {
        RETURN_IF_STOPPED();
        auto pubkey = to_chunk(current_block_.public_key);
        if (!witness::get().verify_signer(pubkey, header.number)) {
            return error::witness_mismatch;
        }
    }

    return error::success;
}

code validate_block::check_block(blockchain::block_chain_impl& chain) const
{
    // These are checks that are independent of the blockchain
    // that can be validated before saving an orphan block.

    const auto& transactions = current_block_.transactions;

    if (transactions.empty() || current_block_.serialized_size() > max_block_size)
        return error::size_limits;

    const auto& header = current_block_.header;

    if (header.is_proof_of_stake()) {
        if (!verify_stake(current_block_)) {
            return error::proof_of_stake;
        }
    }
    else if (header.is_proof_of_work()) {
        if (!check_work(current_block_)) {
            return error::proof_of_work;
        }
    }
    else if (header.is_proof_of_dpos()) {
        if (!check_work(current_block_)) {
            return error::proof_of_work;
        }
        if (!can_use_dpos(header.number)) {
            return error::block_version_not_match;
        }
    }
    else {
        return error::old_version_block;
    }

    RETURN_IF_STOPPED();

    if (!check_max_successive_height(header.number, (chain::block_version)header.version)) {
        return error::block_intermix_interval_error;
    }

    //TO.FIX.CHENHAO.Reject
    const auto&& block_hash = header.hash();
    if (!testnet_ && !config::checkpoint::validate(block_hash, current_block_.header.number, checkpoints_)) {
        return error::checkpoints_failed;
    }

    chain::header prev_header = fetch_block(height_ - 1);
    if (current_block_.header.number >= future_blocktime_fork_height) {
        // 未来区块时间攻击分叉，执行新规则检查
        if (current_block_.header.number >= pos_enabled_height) {
            if (!check_time_stamp(header.timestamp, asio::seconds(block_timespan_window))) {
                return error::futuristic_timestamp;
            }
        }
        else if (!check_time_stamp(header.timestamp, time_stamp_window_future_blocktime_fix)) {
            return error::futuristic_timestamp;
        }

        // 过去区块时间检查
        if (current_block_.header.timestamp < prev_header.timestamp) {
            return error::timestamp_too_early;
        }
    }
    else {
        if (!check_time_stamp(header.timestamp, time_stamp_window)) {
            return error::futuristic_timestamp;
        }
    }

    RETURN_IF_STOPPED();

    if (!check_block_signature(chain)) {
        return error::block_signature_invalid;
    }

    RETURN_IF_STOPPED();

    bool check_genesis_tx = header.is_proof_of_stake() && !chain.pos_exist_before(header.number);
    auto ec = check_coinbase(prev_header, check_genesis_tx);
    if (ec) {
        return ec;
    }

    RETURN_IF_STOPPED();

    if (!is_distinct_tx_set(transactions))
    {
        log::warning(LOG_BLOCKCHAIN) << "is_distinct_tx_set!!!";
        return error::duplicate;
    }

    RETURN_IF_STOPPED();

    const auto sigops = transaction::legacy_sigops_count(transactions);
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

// BUGBUG: we should confirm block hash doesn't exist.
code validate_block::accept_block() const
{
    const auto& header = current_block_.header;
    if (!header.is_proof_of_dpos() && header.bits != work_required(testnet_))
        return error::incorrect_proof_of_work;

    RETURN_IF_STOPPED();

    // Ensure that the block passes checkpoints.
    // This is both DOS protection and performance optimization for sync.
    const auto block_hash = header.hash();
    if (!config::checkpoint::validate(block_hash, height_, checkpoints_))
        return error::checkpoints_failed;

    RETURN_IF_STOPPED();

    // Reject blocks that are below the minimum version for the current height.
    if (!is_valid_version())
        return error::old_version_block;

    return error::success;
}

u256 validate_block::work_required(bool is_testnet) const
{
    uint32_t version = current_block_.header.version;
    chain::header prev_header = fetch_block(height_ - 1);
    header::ptr last_header = get_last_block_header(prev_header, version);
    header::ptr llast_header;
    if (last_header && last_header->number > 2) {
        auto height = last_header->number - 1;
        chain::header prev_last_header = fetch_block(height);
        llast_header = get_last_block_header(prev_last_header, version);
    }

    return HeaderAux::calculate_difficulty(
        current_block_.header, last_header, llast_header);
}

bool validate_block::is_valid_coinbase_height(uint64_t height, const block& block, uint64_t index)
{
    // There must be a transaction with an input.
    if (block.transactions.size() < index + 1 || block.transactions[index].inputs.empty()) {
        return false;
    }

    const auto& actual_script = block.transactions[index].inputs.front().script;

    const script_number number(height);
    const auto height_data = number.data();

    // Get the serialized coinbase input script as a byte vector.
    const auto actual = actual_script.to_data(false);

    // Create the expected script as a byte vector.
    script expected_script;
    expected_script.operations.push_back({ opcode::special, height_data });
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
    uint64_t total_sigops = 0;
    const auto count = transactions.size();
    uint32_t version = current_block_.header.version;
    bool is_pos = current_block_.header.is_proof_of_stake();
    uint64_t coinage_reward_coinbase_index = !is_pos ? 1 : 2;
    uint64_t get_coinage_reward_tx_count = 0;

    ////////////// TODO: parallelize. //////////////
    for (uint64_t tx_index = 0; tx_index < count; ++tx_index)
    {
        auto is_coinstake = false;

        uint64_t value_in = 0;
        const auto& tx = transactions[tx_index];

        RETURN_IF_STOPPED();

        // Count sigops for coinbase tx, but no other checks.
        if (tx.is_coinbase())
            continue;

        if (version == block_version_pos){
            if (tx_index == 1 && tx.is_coinstake()){
                is_coinstake = true;
            }
        }

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

        if (!validate_transaction::tally_fees(chain, tx, value_in, fees, is_coinstake))
        {
            err_tx = tx.hash();
            return error::fees_out_of_range;
        }
    }

    auto check_reward_count = [=] {
        uint64_t coinage_reward_count = BC_MAX_SIZE;
        switch (version){
        case block_version_pow:
        case block_version_dpos:
            coinage_reward_count = coinage_reward_coinbase_index - 1;
            break;
        case block_version_pos:
            coinage_reward_count = coinage_reward_coinbase_index - 2;
            break;
        }
        return coinage_reward_count == get_coinage_reward_tx_count;
    };

    if (!check_reward_count()) {
        return error::invalid_coinage_reward_coinbase;
    }

    RETURN_IF_STOPPED();

    const auto& coinbase = transactions.front();
    const auto reward = coinbase.total_output_value();
    const auto value = consensus::miner::calculate_block_subsidy(height_, testnet_, version) + fees;
    if (reward > value) {
        log::error(LOG_BLOCKCHAIN) << "ETP coinbase is too large! " << reward << " VS " << value;
        return error::coinbase_too_large;
    }

    if (coinbase.outputs.size() > 1) {
        RETURN_IF_STOPPED();

        if (!consensus::witness::is_begin_of_epoch(height_)) {
            const auto& coinbase_mst_output = coinbase.outputs[1];
            auto mst_reward = coinbase_mst_output.get_asset_amount();
            auto symbol = coinbase_mst_output.get_asset_symbol();

            auto mining_asset = chain.get_issued_blockchain_asset(symbol);
            if (nullptr == mining_asset) {
                log::error(LOG_BLOCKCHAIN) << "MST " << symbol << " for mining does not exist.";
                return error::mst_coinbase_invalid;
            }

            auto mining_cert = chain.get_asset_cert(symbol, asset_cert_ns::mining);
            if (!mining_cert) {
                log::error(LOG_BLOCKCHAIN) << "Mining MST " << symbol << " is not allowed.";
                return error::mst_coinbase_invalid;
            }

            auto mst_value = consensus::miner::calculate_mst_subsidy(
                *mining_asset, *mining_cert, height_, testnet_, version);
            if (mst_reward > mst_value) {
                log::error(LOG_BLOCKCHAIN) << "MST coinbase is too large! " << mst_reward << " VS " << mst_value;
                return error::mst_coinbase_too_large;
            }
        }
    }

    RETURN_IF_STOPPED();

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
            ec = validate_tx->check_transaction_connect_input(current_block_.header.number);
        }

        for (uint64_t i = 0; (!ec) && (i < tx.outputs.size()); ++i) {
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

    return error::success;
}

bool validate_block::check_block_signature(blockchain::block_chain_impl& chain) const
{
    const auto& header = current_block_.header;
    const auto is_proof_of_stake = header.is_proof_of_stake();
    const auto is_proof_of_dpos = header.is_proof_of_dpos();

    if (!(is_proof_of_stake || is_proof_of_dpos)) {
        return true;
    }

    if (is_proof_of_stake && !is_coin_stake(current_block_)) {
        log::error(LOG_BLOCKCHAIN) << "Invalid coinstake tx!";
        return false;
    }

    const auto& blocksig = current_block_.blocksig;
    if (blocksig.empty()) {
        log::error(LOG_BLOCKCHAIN) << "Miss "
            << (is_proof_of_stake ? "PoS" : "DPoS") << " blocksig!";
        return false;
    }

    bool result = true;
    if (is_proof_of_stake) {
        BITCOIN_ASSERT(current_block_.transactions.size() > 1);
        const auto& coinstake_tx = current_block_.transactions[1];
        const auto& pubkey_data = coinstake_tx.inputs[0].script.operations.back().data;
        result = verify_signature(pubkey_data, header.hash(), blocksig);
    }

    if (is_proof_of_dpos) {
        const auto& ec_pubkey = current_block_.public_key;
        if (ec_pubkey.empty()) {
            log::error(LOG_BLOCKCHAIN) << "Miss DPoS public_key";
            return false;
        }
        result = verify_signature(ec_pubkey, header.hash(), blocksig);
    }

    return result;
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
                                     uint64_t index_in_parent, uint64_t& value_in, uint64_t& total_sigops) const
{
    BITCOIN_ASSERT(!tx.is_coinbase());

    ////////////// TODO: parallelize. //////////////
    for (uint64_t input_index = 0; input_index < tx.inputs.size(); ++input_index)
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

bool validate_block::script_hash_signature_operations_count(uint64_t& out_count,
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

    out_count = operation::count_script_sigops(eval_script.operations, true);
    return true;
}

bool validate_block::get_transaction(const hash_digest& tx_hash,
                                     chain::transaction& prev_tx, uint64_t& prev_height) const
{
    return fetch_transaction(prev_tx, prev_height, tx_hash);
}

bool validate_block::get_header(chain::header& out_header, uint64_t height) const
{
    out_header = fetch_block(height);
    return true;
}

bool validate_block::connect_input(uint64_t index_in_parent,
                                   const transaction& current_tx, uint64_t input_index, uint64_t& value_in,
                                   uint64_t& total_sigops) const
{
    BITCOIN_ASSERT(input_index < current_tx.inputs.size());

    // Lookup previous output
    uint64_t previous_height;
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
    uint64_t count;
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
        log::warning(LOG_BLOCKCHAIN) << "Input money exceeds 21 million.";
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
