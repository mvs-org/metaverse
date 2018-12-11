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
#include <metaverse/blockchain/validate_block_impl.hpp>
#include <metaverse/consensus/miner.hpp>

#include <cstddef>
#include <metaverse/bitcoin.hpp>
#include <metaverse/blockchain/block_detail.hpp>
#include <metaverse/consensus/miner/MinerAux.h>
#include <metaverse/blockchain/block_chain_impl.hpp>

namespace libbitcoin {
namespace blockchain {

// Value used to define median time past.
static constexpr size_t median_time_past_blocks = 11;

validate_block_impl::validate_block_impl(block_chain_impl& chain,
        size_t fork_index, const block_detail::list& orphan_chain,
        size_t orphan_index, size_t height, const chain::block& block,
        bool testnet, const config::checkpoint::list& checks,
        stopped_callback stopped)
    : validate_block(height, block, testnet, checks, stopped),
      chain_(chain),
      height_(height),
      fork_index_(fork_index),
      orphan_index_(orphan_index),
      orphan_chain_(orphan_chain)
{
}

bool validate_block_impl::check_work(const chain::block& block) const
{
    chain::header parent_header = fetch_block(block.header.number - 1);

    if (block.is_proof_of_dpos()) {
        return block.header.bits == parent_header.bits;
    }

    chain::header::ptr last_header = get_last_block_header(parent_header, block.is_proof_of_stake());
    BITCOIN_ASSERT(nullptr != last_header);

    return MinerAux::verify_work(block.header, last_header);
}

bool validate_block_impl::verify_stake(const chain::block& block) const
{
    // check stake txs
    const auto& txs = block.transactions;
    if(!is_coin_stake(block)){
        log::error(LOG_BLOCKCHAIN)
            << "Failed to check stake, invalid coinstake. height: "
            << block.header.number << ", " << txs.size() << " txs.";
        return false;
    }

    // check stake
    const auto& coinstake = txs[1];
    const auto& stake_output_point = coinstake.inputs[0].previous_output;
    bc::wallet::payment_address pay_address(coinstake.inputs[0].get_script_address());

    auto height = block.header.number - 1;
    if (!chain_.check_pos_capability(height, pay_address, false)) {
        log::error(LOG_BLOCKCHAIN)
            << "Failed to check pos capability. height: "
            << block.header.number << ", address=" << pay_address.encoded()
            << ", "<< txs.size() << " txs.";
        return false;
    }

    uint64_t utxo_height = 0;
    chain::transaction utxo_tx;
    if (!fetch_transaction(utxo_tx, utxo_height, stake_output_point.hash)) {
        log::error(LOG_BLOCKCHAIN)
            << "Failed to check stake, can not get stake transaction "
            << encode_hash(stake_output_point.hash);
        return false;
    }

    BITCOIN_ASSERT(stake_output_point.index < utxo_tx.outputs.size());

    if (!chain_.check_pos_utxo_capability(
        block.header.number, utxo_tx, stake_output_point.index, utxo_height, true, this)) {
        log::error(LOG_BLOCKCHAIN)
            << "Failed to check utxo capability, hash=" << encode_hash(stake_output_point.hash)
            << ", index=" << stake_output_point.index
            << ", utxo height=" << utxo_height;

        return false;
    }

    auto stake_output = utxo_tx.outputs.at(stake_output_point.index);
    chain::output_info stake_info = {stake_output, stake_output_point, utxo_height};

    return MinerAux::verify_stake(block.header, stake_info);
}

bool validate_block_impl::is_coin_stake(const chain::block& block) const
{
    const auto& txs = block.transactions;
    if (txs.size() < 2 || !txs[0].is_coinbase() || !txs[1].is_coinstake()) {
        return false;
    }

    return true;
}

u256 validate_block_impl::previous_block_bits() const
{
    // Read block header (top - 1) and return bits
    return fetch_block(height_ - 1).bits;
}

validate_block::versions validate_block_impl::preceding_block_versions(
    size_t maximum) const
{
    // 1000 previous versions maximum sample.
    // 950 previous versions minimum required for enforcement.
    // 750 previous versions minimum required for activation.
    const auto size = std::min(maximum, height_);

    // Read block (top - 1) through (top - 1000) and return version vector.
    versions result;
    for (size_t index = 0; index < size; ++index)
    {
        const auto version = fetch_block(height_ - index - 1).version;

        // Some blocks have high versions, see block #390777.
        static const auto maximum = static_cast<uint32_t>(max_uint8);
        const auto normal = std::min(version, maximum);
        result.push_back(static_cast<uint8_t>(normal));
    }

    return result;
}

uint64_t validate_block_impl::actual_time_span(size_t interval) const
{
    BITCOIN_ASSERT(height_ > 0 && height_ >= interval);

    // height - interval and height - 1, return time difference
    return fetch_block(height_ - 1).timestamp - fetch_block(height_ - interval).timestamp;
}

uint64_t validate_block_impl::median_time_past() const
{
    // Read last 11 (or height if height < 11) block times into array.
    const auto count = std::min(height_, median_time_past_blocks);

    std::vector<uint64_t> times;
    for (size_t i = 0; i < count; ++i)
        times.push_back(fetch_block(height_ - i - 1).timestamp);

    // Sort and select middle (median) value from the array.
    std::sort(times.begin(), times.end());
    return times.empty() ? 0 : times[times.size() / 2];
}

chain::header validate_block_impl::fetch_block(size_t fetch_height) const
{
    if (fetch_height > fork_index_) {
        const auto fetch_index = fetch_height - fork_index_ - 1;
        BITCOIN_ASSERT(fetch_index <= orphan_index_);
        BITCOIN_ASSERT(orphan_index_ < orphan_chain_.size());
        return orphan_chain_[fetch_index]->actual()->header;
    }

    chain::header out;
    DEBUG_ONLY(const auto result = ) chain_.get_header(out, fetch_height);
    BITCOIN_ASSERT(result);
    return out;
}

chain::header::ptr validate_block_impl::get_last_block_header(const chain::header& parent_header, bool is_staking) const
{
    uint64_t height = parent_header.number;
    if (parent_header.is_proof_of_stake() == is_staking) {
        // log::info(LOG_BLOCKCHAIN) << "validate_block_impl::get_last_block_header: prev: "
        //     << std::to_string(parent_header.number) << ", last: " << std::to_string(height);
        return std::make_shared<chain::header>(parent_header);
    }

    while ((is_staking && height > pos_enabled_height) || (!is_staking && height > 2)) {
        chain::header prev_header = fetch_block(--height);
        if (prev_header.is_proof_of_stake() == is_staking) {
            // log::info(LOG_BLOCKCHAIN) << "validate_block_impl::get_last_block_header: prev: "
            //     << std::to_string(parent_header.number) << ", last: " << std::to_string(height);
            return std::make_shared<chain::header>(prev_header);
        }
    }

    return nullptr;
}

bool tx_after_fork(size_t tx_height, size_t fork_index)
{
    return tx_height > fork_index;
}

bool validate_block_impl::transaction_exists(const hash_digest& tx_hash) const
{
    uint64_t out_height;
    chain::transaction unused;
    const auto result = chain_.get_transaction(unused, out_height, tx_hash);
    if (!result)
        return false;

    BITCOIN_ASSERT(out_height <= max_size_t);
    const auto tx_height = static_cast<size_t>(out_height);
    return tx_height <= fork_index_;
}

bool validate_block_impl::is_output_spent(
    const chain::output_point& outpoint) const
{
    hash_digest out_hash;
    const auto result = chain_.get_outpoint_transaction(out_hash, outpoint);
    if (!result)
        return false;

    // Lookup block height. Is the spend after the fork point?
    return transaction_exists(out_hash);
}

bool validate_block_impl::fetch_transaction(chain::transaction& tx,
        size_t& tx_height, const hash_digest& tx_hash) const
{
    uint64_t out_height;
    const auto result = chain_.get_transaction(tx, out_height, tx_hash);

    BITCOIN_ASSERT(out_height <= max_size_t);
    tx_height = static_cast<size_t>(out_height);

    if (!result || tx_after_fork(tx_height, fork_index_)) {
        return fetch_orphan_transaction(tx, tx_height, tx_hash);
    }

    return true;
}

bool validate_block_impl::fetch_orphan_transaction(chain::transaction& tx,
        size_t& tx_height, const hash_digest& tx_hash) const
{
    for (size_t orphan = 0; orphan <= orphan_index_; ++orphan) {
        const auto& orphan_block = orphan_chain_[orphan]->actual();
        for (const auto& orphan_tx : orphan_block->transactions) {
            if (orphan_tx.hash() == tx_hash) {
                // TRANSACTION COPY
                tx = orphan_tx;
                tx_height = fork_index_ + orphan + 1;
                return true;
            }
        }
    }

    return false;
}

std::string validate_block_impl::get_did_from_address_consider_orphan_chain(
    const std::string& address, const std::string& did_symbol) const
{
    BITCOIN_ASSERT(!address.empty());

    if (address.empty()) {
        log::debug("blockchain") << "get_did_from_address_consider_orphan_chain: address is empty";
        return "";
    }

    auto orphan = orphan_index_;
    while (orphan > 0) {
        const auto& orphan_block = orphan_chain_[--orphan]->actual();
        for (const auto& orphan_tx : orphan_block->transactions) {
            // iter outputs
            for (const auto& output : orphan_tx.outputs) {
                if (output.is_did_register() || output.is_did_transfer()) {
                    if (address == output.get_did_address()) {
                        return output.get_did_symbol();
                    }
                }
            }

            // iter inputs
            for (const auto& input : orphan_tx.inputs) {
                size_t previous_height;
                transaction previous_tx;
                const auto& previous_output = input.previous_output;

                // This searches the blockchain and then the orphan pool up to and
                // including the current (orphan) block and excluding blocks above fork.
                if (!fetch_transaction(previous_tx, previous_height, previous_output.hash))
                {
                    log::warning(LOG_BLOCKCHAIN)
                            << "Failure fetching input transaction ["
                            << encode_hash(previous_output.hash) << "]";
                    return "";
                }

                const auto& previous_tx_out = previous_tx.outputs[previous_output.index];

                if (previous_tx_out.is_did_register() || previous_tx_out.is_did_transfer()) {
                    if (address == previous_tx_out.get_did_address()) {
                        return "";
                    }
                }
            }
        }
    }

    return did_symbol;
}

bool validate_block_impl::is_did_match_address_in_orphan_chain(const std::string& did, const std::string& address) const
{
    BITCOIN_ASSERT(!did.empty());
    BITCOIN_ASSERT(!address.empty());

    if (address.empty()) {
        log::debug("blockchain") << "check did match address in orphan chain: address is null for did: " << did;
        return false;
    }

    auto orphan = orphan_index_;
    while (orphan > 0) {
        const auto& orphan_block = orphan_chain_[--orphan]->actual();
        for (const auto& orphan_tx : orphan_block->transactions) {
            for (auto& output : orphan_tx.outputs) {
                if (output.is_did_register() || output.is_did_transfer()) {
                    if (did == output.get_did_symbol()) {
                        return address == output.get_did_address();
                    }
                }
            }
        }
    }

    return false;
}

bool validate_block_impl::is_did_in_orphan_chain(const std::string& did) const
{
    BITCOIN_ASSERT(!did.empty());

    for (size_t orphan = 0; orphan < orphan_index_; ++orphan) {
        const auto& orphan_block = orphan_chain_[orphan]->actual();
        for (const auto& orphan_tx : orphan_block->transactions) {
            for (auto& output : orphan_tx.outputs) {
                if (output.is_did_register()) {
                    if (did == output.get_did_symbol()) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool validate_block_impl::is_asset_in_orphan_chain(const std::string& symbol) const
{
    BITCOIN_ASSERT(!symbol.empty());

    for (size_t orphan = 0; orphan < orphan_index_; ++orphan) {
        const auto& orphan_block = orphan_chain_[orphan]->actual();
        for (const auto& orphan_tx : orphan_block->transactions) {
            for (const auto& output : orphan_tx.outputs) {
                if (output.is_asset_issue()) {
                    if (symbol == output.get_asset_symbol()) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool validate_block_impl::is_asset_cert_in_orphan_chain(const std::string& symbol, asset_cert_type cert_type) const
{
    BITCOIN_ASSERT(!symbol.empty());

    for (size_t orphan = 0; orphan < orphan_index_; ++orphan) {
        const auto& orphan_block = orphan_chain_[orphan]->actual();
        for (const auto& orphan_tx : orphan_block->transactions) {
            for (const auto& output : orphan_tx.outputs) {
                if (symbol == output.get_asset_cert_symbol() &&
                    cert_type == output.get_asset_cert_type()) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool validate_block_impl::is_asset_mit_in_orphan_chain(const std::string& symbol) const
{
    BITCOIN_ASSERT(!symbol.empty());

    for (size_t orphan = 0; orphan < orphan_index_; ++orphan) {
        const auto& orphan_block = orphan_chain_[orphan]->actual();
        for (const auto& orphan_tx : orphan_block->transactions) {
            for (const auto& output : orphan_tx.outputs) {
                if (output.is_asset_mit_register()) {
                    if (symbol == output.get_asset_mit_symbol()) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool validate_block_impl::is_output_spent(
    const chain::output_point& previous_output,
    size_t index_in_parent, size_t input_index) const
{
    // Search for double spends. This must be done in both chain AND orphan.
    // Searching chain when this tx is an orphan is redundant but it does not
    // happen enough to care.
    if (is_output_spent(previous_output))
        return true;

    if (orphan_is_spent(previous_output, index_in_parent, input_index))
        return true;

    return false;
}

bool validate_block_impl::orphan_is_spent(
    const chain::output_point& previous_output,
    size_t skip_tx, size_t skip_input) const
{
    for (size_t orphan = 0; orphan <= orphan_index_; ++orphan) {
        const auto& orphan_block = orphan_chain_[orphan]->actual();
        const auto& transactions = orphan_block->transactions;

        BITCOIN_ASSERT(!transactions.empty());
        BITCOIN_ASSERT(transactions.front().is_coinbase());

        for (size_t tx_index = 0; tx_index < transactions.size(); ++tx_index) {
            // TODO: too deep, move this section to subfunction.
            const auto& orphan_tx = transactions[tx_index];

            for (size_t input_index = 0; input_index < orphan_tx.inputs.size(); ++input_index) {
                const auto& orphan_input = orphan_tx.inputs[input_index];

                if (orphan == orphan_index_ && tx_index == skip_tx && input_index == skip_input)
                    continue;

                if (orphan_input.previous_output == previous_output)
                    return true;
            }
        }
    }

    return false;
}

bool validate_block_impl::check_get_coinage_reward_transaction(const chain::transaction& coinage_reward_coinbase, const chain::output& output) const
{
    uint64_t lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(output.script.operations);
    uint64_t coinbase_lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(coinage_reward_coinbase.outputs[0].script.operations);
    wallet::payment_address addr1 = wallet::payment_address::extract(coinage_reward_coinbase.outputs[0].script);
    wallet::payment_address addr2 = wallet::payment_address::extract(output.script);
    uint64_t coinage_reward_value = libbitcoin::consensus::miner::calculate_lockblock_reward(lock_height, output.value);

    if (addr1 == addr2
            && lock_height == coinbase_lock_height
            && coinage_reward_value == coinage_reward_coinbase.outputs[0].value) {
        return true;
    }
    else {
        return false;
    }
}

} // namespace blockchain
} // namespace libbitcoin
