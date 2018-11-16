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
#include <metaverse/blockchain/organizer.hpp>
#include <metaverse/blockchain/block.hpp>

#include <boost/thread.hpp>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <numeric>
#include <metaverse/bitcoin.hpp>
#include <metaverse/blockchain/block_detail.hpp>
#include <metaverse/blockchain/orphan_pool.hpp>
#include <metaverse/blockchain/organizer.hpp>
#include <metaverse/blockchain/settings.hpp>
#include <metaverse/blockchain/validate_block_impl.hpp>
#include <metaverse/bitcoin/chain/header.hpp>
#include <metaverse/node/protocols/protocol_block_out.hpp>
#include <metaverse/bitcoin/wallet/payment_address.hpp>
#include <metaverse/bitcoin/formats/base_16.hpp>
#include <metaverse/consensus/witness.hpp>

namespace libbitcoin {
namespace blockchain {

using namespace bc::chain;
using namespace bc::config;

#define NAME "organizer"

organizer::organizer(threadpool& pool, block_chain_impl& chain,
    const settings& settings)
  : stopped_(true),
    use_testnet_rules_(settings.use_testnet_rules),
    checkpoints_(checkpoint::sort(settings.checkpoints)),
    chain_(chain),
    orphan_pool_(settings.block_pool_capacity),
    subscriber_(std::make_shared<reorganize_subscriber>(pool, NAME))
{
}

void organizer::start()
{
    // update witness list
    {
        chain_.set_sync_disabled(true);
        unique_lock lock(chain_.get_mutex());
        uint64_t height = 0;
        if (!chain_.get_last_height(height)) {
            log::info(LOG_BLOCKCHAIN) << "get last height failed";
            return;
        }
        log::info(LOG_BLOCKCHAIN) << "begin to update witness list at height " << height;
        if (!consensus::witness::get().update_witness_list(height)) {
            log::info(LOG_BLOCKCHAIN) << "update witness list failed at height " << height;
            return;
        }
        log::info(LOG_BLOCKCHAIN) << consensus::witness::get().show_list();
        chain_.set_sync_disabled(false);
    }

    stopped_ = false;
    subscriber_->start();
}

void organizer::stop()
{
    stopped_ = true;
    subscriber_->stop();
    subscriber_->invoke(error::service_stopped, 0, {}, {});
}

bool organizer::stopped()
{
    return stopped_;
}

uint64_t organizer::count_inputs(const chain::block& block)
{
    const auto value = [](uint64_t total, const transaction& tx)
    {
        return total + tx.inputs.size();
    };

    const auto& txs = block.transactions;
    return std::accumulate(txs.begin(), txs.end(), uint64_t(0), value);
}

bool organizer::strict(uint64_t fork_point) const
{
    return checkpoints_.empty() || fork_point >= checkpoints_.back().height();
}

// This verifies the block at orphan_chain[orphan_index]->actual()
code organizer::verify(uint64_t fork_point,
    const block_detail::list& orphan_chain, uint64_t orphan_index)
{
    if (stopped())
        return error::service_stopped;

    BITCOIN_ASSERT(orphan_index < orphan_chain.size());
    const auto& current_block = orphan_chain[orphan_index]->actual();
    const auto height = fork_point + orphan_index + 1;
    BITCOIN_ASSERT(height != 0);

    const auto callback = [this]()
    {
        return stopped();
    };

    // Validates current_block
    validate_block_impl validate(chain_, fork_point, orphan_chain, orphan_index, height,
        *current_block, use_testnet_rules_, checkpoints_, callback);

    // Checks that are independent of the chain.
    auto ec = validate.check_block(chain_);
    if (error::success != ec.value()) {
        log::debug(LOG_BLOCKCHAIN) << "organizer: check_block failed! error:"
            << std::to_string(ec.value()) << ", fork_point: "
            << std::to_string(fork_point) << ", orphan_index: " << std::to_string(orphan_index);
        return ec;
    }

    // Checks that are dependent on height and preceding blocks.
    ec = validate.accept_block();
    if (error::success != ec.value()) {
        log::debug(LOG_BLOCKCHAIN) << "organizer: accept_block failed! error:"
            << std::to_string(ec.value()) << ", fork_point: "
            << std::to_string(fork_point) << ", orphan_index: " << std::to_string(orphan_index);
        return ec;
    }

    // Start strict validation if past last checkpoint.
    if (!strict(fork_point)) {
        return ec;
    }

    const auto total_inputs = count_inputs(*current_block);
    const auto total_transactions = current_block->transactions.size();

    log::info(LOG_BLOCKCHAIN)
        << "Block [" << height << "] version (" << std::to_string(current_block->header.version)
        << ") verify (" << total_transactions
        << ") txs and (" << total_inputs << ") inputs";

    // Time this for logging.
    const auto timed = [this, &ec, &validate]()
    {
        hash_digest err_tx;
        // Checks that include input->output traversal.
        ec = validate.connect_block(err_tx, chain_);
        if(ec && err_tx != null_hash) {
            chain_.pool().delete_tx(err_tx);
        }

    };

    // Execute the timed validation.
    const auto elapsed = timer<std::chrono::milliseconds>::duration(timed);
    const auto ms_per_block = static_cast<float>(elapsed.count());
    const auto ms_per_input = ms_per_block / total_inputs;
    const auto seconds_per_block = ms_per_block / 1000;
    const auto verified = ec ? "unverified" : "verified";

    log::info(LOG_BLOCKCHAIN)
        << "Block [" << height << "] version (" << std::to_string(current_block->header.version)
        << ") " << verified << " in ("
        << seconds_per_block << ") secs or (" << ms_per_input << ") ms/input";

    return ec;
}

// This is called on every block_chain_impl::do_store() call.
void organizer::organize()
{
    process_queue_ = orphan_pool_.unprocessed();

    while (!process_queue_.empty() && !stopped())
    {
        const auto process_block = process_queue_.back();
        process_queue_.pop_back();
        process(process_block);
    }
}

bool organizer::add(block_detail::ptr block)
{
    return orphan_pool_.add(block);
}

void organizer::filter_orphans(message::get_data::ptr message)
{
    orphan_pool_.filter(message);
}

void organizer::process(block_detail::ptr process_block)
{
    BITCOIN_ASSERT(process_block);

    std::vector<block_detail::ptr> blocks;
    blocks.push_back(process_block);

    while(blocks.empty() == false)
    {
        process_block = blocks.back();
        blocks.pop_back();
        // Trace the chain in the orphan pool
        auto orphan_chain = orphan_pool_.trace(process_block);
        BITCOIN_ASSERT(orphan_chain.size() >= 1);

        uint64_t fork_index;
        const auto& hash = orphan_chain.front()->actual()->
        header.previous_block_hash;

        // Verify the blocks in the orphan chain.
        if (chain_.get_height(fork_index, hash))
        {
            uint64_t current_block_height = 0;
            DEBUG_ONLY(auto ok =) chain_.get_last_height(current_block_height);
            BITCOIN_ASSERT(ok);
            auto witness_list = consensus::witness::get().get_witness_list();
            auto candidate_list = consensus::witness::get().get_candidate_list();
            if (consensus::witness::get_vote_result_height(fork_index) !=
                consensus::witness::get_vote_result_height(current_block_height)) {
                consensus::witness::get().update_witness_list(fork_index);
            }

            auto ret = replace_chain(fork_index, orphan_chain);

            const auto& num_of_poped_blocks = std::get<0>(ret);
            const auto& num_of_pushed_blocks = std::get<1>(ret);
            const auto need_recovery = num_of_poped_blocks == 0 && num_of_pushed_blocks == 0;
            const auto need_reupdate = num_of_poped_blocks != 0 && num_of_pushed_blocks == 0;
            if (need_recovery) {
                consensus::witness::get().swap_witness_list(witness_list);
                consensus::witness::get().swap_candidate_list(candidate_list);
            } else if (need_reupdate) {
                const auto& new_block_height = std::get<2>(ret);
                if (consensus::witness::get_vote_result_height(fork_index) !=
                    consensus::witness::get_vote_result_height(new_block_height)) {
                    consensus::witness::get().update_witness_list(new_block_height);
                }
            }

            if(orphan_chain.empty() == false)
            {
                const auto hash = orphan_chain.back()->actual()->header.hash();
                block_detail::ptr block;
                while((block = orphan_pool_.delete_pending_block(hash)))
                {
                    blocks.push_back(block);
                    log::warning(LOG_BLOCKCHAIN) << "pop pendingblock hash:" << encode_hash(hash) << " process_block hash:" << encode_hash(block->actual()->header.hash());
                }
            }
        }
        else
        {
            orphan_pool_.add_pending_block(hash, orphan_chain.back());
            log::warning(LOG_BLOCKCHAIN) << "push pendingblock hash:" << encode_hash(hash) << " process_block hash:" << encode_hash(orphan_chain.back()->actual()->header.hash());
        }

        // Don't mark all orphan_chain as processed here because there might be
        // a winning fork from an earlier block.
        process_block->set_processed();
    }
}

/*********************************************************************
 * this set include blocks which cause consensus validation problems.
 ********************************************************************/
// the pair's structure: first is block height, second is block hash
static std::set<std::pair<uint64_t, std::string>> exception_blocks {
    // this block has error of merkle_mismatch,
    // because it exist a too long memo text which is more than 300 bytes.
    // memo text length should be less than 256 bytes limited by the database record design.
    // we will add this length verification in the transaction validation after nova version.
    {1211234, "3a17c696ba0d506b07e85b8440a99d868ae93c985064eaf4c616d13911bd97cb"},
    // this block does not satisfy new consensus verify of input/output
    {1258192, "d7d5d80c8cb760b794f156c36b519ad9b4b10b9dfcd4e123c8f54be3c71432d3"}
};

// Return a tuple <number of poped blocks, number of pushed blocks, current block height>
std::tuple<uint64_t, uint64_t, uint64_t>
organizer::replace_chain(uint64_t fork_index, block_detail::list& orphan_chain)
{
    auto ret = std::make_tuple<uint64_t, uint64_t, uint64_t>(0, 0, 0);
    auto& num_of_poped_blocks = std::get<0>(ret);
    auto& num_of_pushed_blocks = std::get<1>(ret);
    auto& current_block_height = std::get<2>(ret);

    u256 orphan_work = 0;

    for (uint64_t orphan = 0; orphan < orphan_chain.size(); ++orphan)
    {
        // This verifies the block at orphan_chain[orphan]->actual()
        if(!orphan_chain[orphan]->get_is_checked_work_proof())
        {
            const auto ec = verify(fork_index, orphan_chain, orphan);
            if (ec)
            {
                // If invalid block info is also set for the block.
                if (ec.value() != error::service_stopped)
                {
                    const auto& header = orphan_chain[orphan]->actual()->header;
                    const auto block_hash = encode_hash(header.hash());

                    if (exception_blocks.count(std::make_pair(header.number, block_hash)))
                    {
                        orphan_chain[orphan]->set_is_checked_work_proof(true);
                        const auto& orphan_block = orphan_chain[orphan]->actual();
                        orphan_work += block_work(orphan_block->header.bits);
                        continue;
                    }

                    log::warning(LOG_BLOCKCHAIN)
                        << "Invalid block [" << block_hash << "] " << ec.message();
                }

                // Block is invalid, clip the orphans.
                clip_orphans(orphan_chain, orphan, ec);
                if(orphan_chain.empty())
                {
                    log::warning(LOG_BLOCKCHAIN) << "orphan_chain.empty()";
                    return ret;
                }

                // Stop summing work once we discover an invalid block
                break;
            }
            else
            {
                orphan_chain[orphan]->set_is_checked_work_proof(true);
            }
        }

        const auto& orphan_block = orphan_chain[orphan]->actual();
        orphan_work += block_work(orphan_block->header.bits);
    }

    // All remaining blocks in orphan_chain should all be valid now
    // Compare the difficulty of the 2 forks (original and orphan)
    const auto begin_index = fork_index + 1;

    u256 main_work;
    DEBUG_ONLY(auto result =) chain_.get_difficulty(main_work, begin_index);
    BITCOIN_ASSERT(result);

    delete_fork_chain_hash(orphan_chain[orphan_chain.size() - 1]->actual()->header.previous_block_hash);
    if (orphan_work <= main_work)
    {
        if(orphan_chain.size() % node::locator_cap  == 0)
            orphan_chain.back()->set_error(error::fetch_more_block);
        add_fork_chain_hash(orphan_chain.back()->actual()->header.hash());

        log::debug(LOG_BLOCKCHAIN)
            << "Insufficient work to reorganize at [" << begin_index << "]" << "orphan_work:" << orphan_work << " main_work:" << main_work;
        return ret;
    }

    // Replace! Switch!
    block_detail::list released_blocks;
    auto pop_all_success = chain_.pop_from(released_blocks, begin_index);

    if (!released_blocks.empty()) {
        num_of_poped_blocks = released_blocks.size();
        current_block_height = released_blocks.front()->actual()->header.number - 1;
        if (!pop_all_success) {
            log::warning(LOG_BLOCKCHAIN)
                << " not all blocks poped out successfully from " << begin_index
                << ", only pop backward to " << current_block_height
                << ", so stop replace chain.";
            return ret;
        }
        log::warning(LOG_BLOCKCHAIN)
            << " blockchain fork at height:" << released_blocks.front()->actual()->header.number
            << " begin_index:"  << encode_hash(released_blocks.front()->actual()->header.hash())
            << " size:"  << released_blocks.size();
    }

    // We add the arriving blocks first to the main chain because if
    // we add the blocks being replaced back to the pool first then
    // the we can push the arrival blocks off the bottom of the
    // circular buffer.
    // Then when we try to remove the block from the orphans pool,
    // if will fail to find it. I would rather not add an exception
    // there so that problems will show earlier.
    // All arrival_blocks should be blocks from the pool.
    auto arrival_index = fork_index;

    block_detail::list pushed_blocks;
    for (const auto arrival_block: orphan_chain)
    {
        orphan_pool_.remove(arrival_block);

        // Indicates the block is not an orphan.
        arrival_block->set_height(++arrival_index);

        // THIS IS THE DATABASE BLOCK WRITE AND INDEX OPERATION.
        if(chain_.push(arrival_block) == false)
        {
            log::warning(LOG_BLOCKCHAIN)
                << " push block height:" << arrival_block->actual()->header.number
                << " hash:"  << encode_hash(arrival_block->actual()->header.hash())
                << " failed";
            // if push block failed, stop replace
            break;
        }
        else
        {
            current_block_height = arrival_block->actual()->header.number;
            pushed_blocks.push_back(arrival_block);
            log::debug(LOG_BLOCKCHAIN)
                << " push block height:" << arrival_block->actual()->header.number
                << " hash:"  << encode_hash(arrival_block->actual()->header.hash())
                << " succeed";
        }
    }

    num_of_pushed_blocks = pushed_blocks.size();

    // Add the old blocks back to the pool (as processed with orphan height).
    for (const auto replaced_block: released_blocks)
    {
        replaced_block->set_processed();
        orphan_pool_.add(replaced_block);

        log::warning(LOG_BLOCKCHAIN)
            << " blockchain fork old block number:" << replaced_block->actual()->header.number
            << " hash_index:"  << encode_hash(replaced_block->actual()->header.hash())
            << " miner address:"  << wallet::payment_address::extract(replaced_block->actual()->transactions[0].outputs[0].script);
        for(auto& tx : replaced_block->actual()->transactions)
        {
            log::warning(LOG_BLOCKCHAIN) << " forked transaction hash:" << encode_hash(tx.hash()) << " data:" << tx.to_string(0);
        }
    }

    notify_reorganize(fork_index, pushed_blocks, released_blocks);
    return ret;
}

void organizer::remove_processed(block_detail::ptr remove_block)
{
    const auto it = std::find(process_queue_.begin(), process_queue_.end(),
        remove_block);

    if (it != process_queue_.end())
        process_queue_.erase(it);
}

void organizer::clip_orphans(block_detail::list& orphan_chain,
    uint64_t orphan_index, const code& invalid_reason)
{
    // Remove from orphans pool and process queue.
    auto orphan_start = orphan_chain.begin() + orphan_index;

    for (auto it = orphan_start; it != orphan_chain.end(); ++it)
    {
        if (it == orphan_start)
            (*it)->set_error(invalid_reason);
        else
            (*it)->set_error(error::previous_block_invalid);

        (*it)->set_processed();
        remove_processed(*it);
        orphan_pool_.remove(*it);
    }

    orphan_chain.erase(orphan_start, orphan_chain.end());
}

void organizer::subscribe_reorganize(reorganize_handler handler)
{
    subscriber_->subscribe(handler, error::service_stopped, 0, {}, {});
}

void organizer::notify_reorganize(uint64_t fork_point,
    const block_detail::list& orphan_chain,
    const block_detail::list& replaced_chain)
{
    const auto to_block_ptr = [](const block_detail::ptr& detail)
    {
        return detail->actual();
    };

    message::block_message::ptr_list arrivals(orphan_chain.size());
    std::transform(orphan_chain.begin(), orphan_chain.end(), arrivals.begin(),
        to_block_ptr);

    message::block_message::ptr_list replacements(replaced_chain.size());
    std::transform(replaced_chain.begin(), replaced_chain.end(),
        replacements.begin(), to_block_ptr);

    subscriber_->relay(error::success, fork_point, arrivals, replacements);
}

void organizer::fired()
{
    subscriber_->relay(error::mock, 0, {}, {});//event to check whether service is stopped
}

std::unordered_map<hash_digest, uint64_t> organizer::get_fork_chain_last_block_hashes()
{
    std::unordered_map<hash_digest, uint64_t> hashes;
    boost::unique_lock<boost::mutex> lock(mutex_fork_chain_last_block_hashes_);
    hashes = fork_chain_last_block_hashes_;
    return hashes;
}

void organizer::add_fork_chain_hash(const hash_digest& hash)
{
    boost::unique_lock<boost::mutex> lock(mutex_fork_chain_last_block_hashes_);
    fork_chain_last_block_hashes_.insert(make_pair(hash, 0));
}

void organizer::delete_fork_chain_hash(const hash_digest& hash)
{
    boost::unique_lock<boost::mutex> lock(mutex_fork_chain_last_block_hashes_);
    fork_chain_last_block_hashes_.erase(hash);
}

} // namespace blockchain
} // namespace libbitcoin
