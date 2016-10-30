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
#include <bitcoin/blockchain/organizer.hpp>
#include <bitcoin/blockchain/block.hpp>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <numeric>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/blockchain/block_detail.hpp>
#include <bitcoin/blockchain/orphan_pool.hpp>
#include <bitcoin/blockchain/organizer.hpp>
#include <bitcoin/blockchain/settings.hpp>
#include <bitcoin/blockchain/simple_chain.hpp>
#include <bitcoin/blockchain/validate_block_impl.hpp>

namespace libbitcoin {
namespace blockchain {

using namespace bc::chain;
using namespace bc::config;

#define NAME "organizer"

organizer::organizer(threadpool& pool, simple_chain& chain,
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
    validate_block_impl validate(chain_, fork_point, orphan_chain,
        orphan_index, height, *current_block, use_testnet_rules_, checkpoints_,
            callback);

    // Checks that are independent of the chain.
    auto ec = validate.check_block();

    if (ec)
        return ec;

    validate.initialize_context();

    // Checks that are dependent on height and preceding blocks.
    ec = validate.accept_block();

    if (ec)
        return ec;

    // Start strict validation if past last checkpoint.
    if (!strict(fork_point))
        return ec;

    const auto total_inputs = count_inputs(*current_block);
    const auto total_transactions = current_block->transactions.size();

    log::info(LOG_BLOCKCHAIN)
        << "Block [" << height << "] verify (" << total_transactions
        << ") txs and (" << total_inputs << ") inputs";

    // Time this for logging.
    const auto timed = [&ec, &validate]()
    {
        // Checks that include input->output traversal.
        ec = validate.connect_block();
    };

    // Execute the timed validation.
    const auto elapsed = timer<std::chrono::milliseconds>::duration(timed);
    const auto ms_per_block = static_cast<float>(elapsed.count());
    const auto ms_per_input = ms_per_block / total_inputs;
    const auto seconds_per_block = ms_per_block / 1000;
    const auto verified = ec ? "unverified" : "verified";

    log::info(LOG_BLOCKCHAIN)
        << "Block [" << height << "] " << verified << " in ("
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

    // Trace the chain in the orphan pool
    auto orphan_chain = orphan_pool_.trace(process_block);
    BITCOIN_ASSERT(orphan_chain.size() >= 1);

    uint64_t fork_index;
    const auto& hash = orphan_chain.front()->actual()->
        header.previous_block_hash;

    // Verify the blocks in the orphan chain.
    if (chain_.get_height(fork_index, hash))
        replace_chain(fork_index, orphan_chain);

    // Don't mark all orphan_chain as processed here because there might be
    // a winning fork from an earlier block.
    process_block->set_processed();
}

void organizer::replace_chain(uint64_t fork_index,
    block_detail::list& orphan_chain)
{
    hash_number orphan_work = 0;

    for (uint64_t orphan = 0; orphan < orphan_chain.size(); ++orphan)
    {
        // This verifies the block at orphan_chain[orphan]->actual()
        const auto ec = verify(fork_index, orphan_chain, orphan);

        if (ec)
        {
            // If invalid block info is also set for the block.
            if (ec != error::service_stopped)
            {
                const auto& header = orphan_chain[orphan]->actual()->header;
                const auto block_hash = encode_hash(header.hash());

                log::warning(LOG_BLOCKCHAIN)
                    << "Invalid block [" << block_hash << "] " << ec.message();
            }

            // Block is invalid, clip the orphans.
            clip_orphans(orphan_chain, orphan, ec);

            // Stop summing work once we discover an invalid block
            break;
        }

        const auto& orphan_block = orphan_chain[orphan]->actual();
        orphan_work += block_work(orphan_block->header.bits);
    }

    // All remaining blocks in orphan_chain should all be valid now
    // Compare the difficulty of the 2 forks (original and orphan)
    const auto begin_index = fork_index + 1;

    hash_number main_work;
    DEBUG_ONLY(auto result =) chain_.get_difficulty(main_work, begin_index);
    BITCOIN_ASSERT(result);

    if (orphan_work <= main_work)
    {
        log::debug(LOG_BLOCKCHAIN)
            << "Insufficient work to reorganize at [" << begin_index << "]";
        return;
    }

    // Replace! Switch!
    block_detail::list released_blocks;
    DEBUG_ONLY(auto success =) chain_.pop_from(released_blocks, begin_index);
    BITCOIN_ASSERT(success);

    if (!released_blocks.empty())
        log::warning(LOG_BLOCKCHAIN)
            << "Reorganizing blockchain [" << begin_index << ", "
            << released_blocks.size() << "]";

    // We add the arriving blocks first to the main chain because if
    // we add the blocks being replaced back to the pool first then
    // the we can push the arrival blocks off the bottom of the
    // circular buffer.
    // Then when we try to remove the block from the orphans pool,
    // if will fail to find it. I would rather not add an exception
    // there so that problems will show earlier.
    // All arrival_blocks should be blocks from the pool.
    auto arrival_index = fork_index;

    for (const auto arrival_block: orphan_chain)
    {
        orphan_pool_.remove(arrival_block);

        // Indicates the block is not an orphan.
        arrival_block->set_height(++arrival_index);

        // THIS IS THE DATABASE BLOCK WRITE AND INDEX OPERATION.
        DEBUG_ONLY(const auto result =) chain_.push(arrival_block);
        BITCOIN_ASSERT(result);
    }

    // Add the old blocks back to the pool (as processed with orphan height).
    for (const auto replaced_block: released_blocks)
    {
        replaced_block->set_processed();
        orphan_pool_.add(replaced_block);
    }

    notify_reorganize(fork_index, orphan_chain, released_blocks);
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

} // namespace blockchain
} // namespace libbitcoin
