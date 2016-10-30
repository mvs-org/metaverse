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
#include <bitcoin/blockchain/block_chain_impl.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <unordered_map>
#include <boost/filesystem.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/database.hpp>
#include <bitcoin/blockchain/block.hpp>
#include <bitcoin/blockchain/block_fetcher.hpp>
#include <bitcoin/blockchain/organizer.hpp>
#include <bitcoin/blockchain/settings.hpp>
#include <bitcoin/blockchain/transaction_pool.hpp>

namespace libbitcoin {
namespace blockchain {

////#define NAME "blockchain"

using namespace bc::chain;
using namespace bc::database;
using namespace boost::interprocess;
using namespace std::placeholders;
using boost::filesystem::path;

block_chain_impl::block_chain_impl(threadpool& pool,
    const blockchain::settings& chain_settings,
    const database::settings& database_settings)
  : stopped_(true),
    settings_(chain_settings),
    organizer_(pool, *this, chain_settings),
    ////read_dispatch_(pool, NAME),
    ////write_dispatch_(pool, NAME),
    transaction_pool_(pool, *this, chain_settings),
    database_(database_settings)
{
}

// Close does not call stop because there is no way to detect thread join.
block_chain_impl::~block_chain_impl()
{
    close();
}

// Utilities.
// ----------------------------------------------------------------------------

static hash_list to_hashes(const block_result& result)
{
    const auto count = result.transaction_count();
    hash_list hashes(count);

    for (size_t index = 0; index < count; ++index)
        hashes.push_back(result.transaction_hash(index));

    return hashes;
}

// Properties.
// ----------------------------------------------------------------------------

transaction_pool& block_chain_impl::pool()
{
    return transaction_pool_;
}

const settings& block_chain_impl::chain_settings() const
{
    return settings_;
}

// Startup and shutdown.
// ----------------------------------------------------------------------------

// Start is required and the blockchain is restartable.
bool block_chain_impl::start()
{
    if (!stopped() || !database_.start())
        return false;

    stopped_ = false;
    organizer_.start();
    transaction_pool_.start();
    return true;
}

// Stop is not required, speeds work shutdown with multiple threads.
bool block_chain_impl::stop()
{
    stopped_ = true;
    organizer_.stop();
    transaction_pool_.stop();
    return database_.stop();
}

// Database threads must be joined before close is called (or destruct).
bool block_chain_impl::close()
{
    return database_.close();
}

// private
bool block_chain_impl::stopped() const
{
    // TODO: consider relying on the database stopped state.
    return stopped_;
}

// Subscriber
// ------------------------------------------------------------------------

void block_chain_impl::subscribe_reorganize(reorganize_handler handler)
{
    // Pass this through to the organizer, which issues the notifications.
    organizer_.subscribe_reorganize(handler);
}

// simple_chain (no locks, not thread safe).
// ----------------------------------------------------------------------------

bool block_chain_impl::get_gap_range(uint64_t& out_first,
    uint64_t& out_last) const
{
    size_t first;
    size_t last;

    if (!database_.blocks.gap_range(first, last))
        return false;

    out_first = static_cast<uint64_t>(first);
    out_last = static_cast<uint64_t>(last);
    return true;
}

bool block_chain_impl::get_next_gap(uint64_t& out_height,
    uint64_t start_height) const
{
    if (stopped())
        return false;

    BITCOIN_ASSERT(start_height <= bc::max_size_t);
    const auto start = static_cast<size_t>(start_height);
    size_t out;

    if (database_.blocks.next_gap(out, start))
    {
        out_height = static_cast<uint64_t>(out);
        return true;
    }

    return false;
}

bool block_chain_impl::get_difficulty(hash_number& out_difficulty,
    uint64_t height) const
{
    size_t top;
    if (!database_.blocks.top(top))
        return false;

    out_difficulty = 0;
    for (uint64_t index = height; index <= top; ++index)
    {
        const auto bits = database_.blocks.get(index).header().bits;
        out_difficulty += block_work(bits);
    }

    return true;
}

bool block_chain_impl::get_header(header& out_header, uint64_t height) const
{
    auto result = database_.blocks.get(height);
    if (!result)
        return false;

    out_header = result.header();
    return true;
}

bool block_chain_impl::get_height(uint64_t& out_height,
    const hash_digest& block_hash) const
{
    auto result = database_.blocks.get(block_hash);
    if (!result)
        return false;

    out_height = result.height();
    return true;
}

bool block_chain_impl::get_last_height(uint64_t& out_height) const
{
    size_t top;
    if (database_.blocks.top(top))
    {
        out_height = static_cast<uint64_t>(top);
        return true;
    }

    return false;
}

bool block_chain_impl::get_outpoint_transaction(hash_digest& out_transaction,
    const output_point& outpoint) const
{
    const auto spend = database_.spends.get(outpoint);
    if (!spend.valid)
        return false;

    out_transaction = spend.hash;
    return true;
}

bool block_chain_impl::get_transaction(transaction& out_transaction,
    uint64_t& out_block_height, const hash_digest& transaction_hash) const
{
    const auto result = database_.transactions.get(transaction_hash);
    if (!result)
        return false;

    out_transaction = result.transaction();
    out_block_height = result.height();
    return true;
}

// This is safe to call concurrently (but with no other methods).
bool block_chain_impl::import(block::ptr block, uint64_t height)
{
    if (stopped())
        return false;

    // THIS IS THE DATABASE BLOCK WRITE AND INDEX OPERATION.
    database_.push(*block, height);
    return true;
}

bool block_chain_impl::push(block_detail::ptr block)
{
    database_.push(*block->actual());
    return true;
}

bool block_chain_impl::pop_from(block_detail::list& out_blocks,
    uint64_t height)
{
    size_t top;

    // The chain has no genesis block, fail.
    if (!database_.blocks.top(top))
        return false;

    BITCOIN_ASSERT_MSG(top <= max_size_t - 1, "chain overflow");

    // The fork is at the top of the chain, nothing to pop.
    if (height == top + 1)
        return true;

    // The fork is disconnected from the chain, fail.
    if (height > top)
        return false;

    // If the fork is at the top there is one block to pop, and so on.
    out_blocks.reserve(top - height + 1);

    for (uint64_t index = top; index >= height; --index)
    {
        const auto block = std::make_shared<block_detail>(database_.pop());
        out_blocks.push_back(block);
    }

    return true;
}

// block_chain (internal locks).
// ----------------------------------------------------------------------------

void block_chain_impl::start_write()
{
    DEBUG_ONLY(const auto result =) database_.begin_write();
    BITCOIN_ASSERT(result);
}

// This call is sequential, but we are preserving the callback model for now.
void block_chain_impl::store(message::block_message::ptr block,
    block_store_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, 0);
        return;
    }

    // We moved write to the network thread using a critical section here.
    // We do not want to give the thread to any other activity at this point.
    // A flood of valid orphans from multiple peers could tie up the CPU here,
    // but resolving that cleanly requires removing the orphan pool.

    ////write_dispatch_.ordered(
    ////    std::bind(&block_chain_impl::do_store,
    ////        this, block, handler));

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section.
    unique_lock lock(mutex_);

    do_store(block, handler);
    ///////////////////////////////////////////////////////////////////////////
}

// This processes the block through the organizer.
void block_chain_impl::do_store(message::block_message::ptr block,
    block_store_handler handler)
{
    start_write();

    // fail fast if the block is already stored...
    if (database_.blocks.get(block->header.hash()))
    {
        stop_write(handler, error::duplicate, 0);
        return;
    }

    const auto detail = std::make_shared<block_detail>(block);

    // ...or if the block is already orphaned.
    if (!organizer_.add(detail))
    {
        stop_write(handler, error::duplicate, 0);
        return;
    }

    // Otherwise organize the chain...
    organizer_.organize();

    //...and then get the particular block's status.
    stop_write(handler, detail->error(), detail->height());
}

///////////////////////////////////////////////////////////////////////////////
// TODO: This should be ordered on channel distacher (modify channel queries).
// This allows channels to run concurrently with internal order preservation.
///////////////////////////////////////////////////////////////////////////////
// This performs a query in the context of the calling thread.
// The callback model is preserved currently in order to limit downstream changes.
// This change allows the caller to manage worker threads.
void block_chain_impl::fetch_serial(perform_read_functor perform_read)
{
    // Post IBD writes are ordered on the strand, so never concurrent.
    // Reads are unordered and concurrent, but effectively blocked by writes.
    const auto try_read = [this, perform_read]()
    {
        const auto handle = database_.begin_read();
        return (!database_.is_write_locked(handle) && perform_read(handle));
    };

    const auto do_read = [this, try_read]()
    {
        // Sleep while waiting for write to complete.
        while (!try_read())
            std::this_thread::sleep_for(asio::milliseconds(10));
    };

    // Initiate serial read operation.
    do_read();
}

////void block_chain_impl::fetch_parallel(perform_read_functor perform_read)
////{
////    // Post IBD writes are ordered on the strand, so never concurrent.
////    // Reads are unordered and concurrent, but effectively blocked by writes.
////    const auto try_read = [this, perform_read]()
////    {
////        const auto handle = database_.begin_read();
////        return (!database_.is_write_locked(handle) && perform_read(handle));
////    };
////
////    const auto do_read = [this, try_read]()
////    {
////        // Sleep while waiting for write to complete.
////        while (!try_read())
////            std::this_thread::sleep_for(std::chrono::milliseconds(10));
////    };
////
////    // Initiate async read operation.
////    read_dispatch_.concurrent(do_read);
////}

////// TODO: This should be ordered on the channel's strand, not across channels.
////void block_chain_impl::fetch_ordered(perform_read_functor perform_read)
////{
////    // Writes are ordered on the strand, so never concurrent.
////    // Reads are unordered and concurrent, but effectively blocked by writes.
////    const auto try_read = [this, perform_read]() -> bool
////    {
////        const auto handle = database_.begin_read();
////        return (!database_.is_write_locked(handle) && perform_read(handle));
////    };
////
////    const auto do_read = [this, try_read]()
////    {
////        // Sleep while waiting for write to complete.
////        while (!try_read())
////            std::this_thread::sleep_for(std::chrono::milliseconds(10));
////    };
////
////    // Initiate async read operation.
////    read_dispatch_.ordered(do_read);
////}

// block_chain (formerly fetch_ordered)
// ----------------------------------------------------------------------------

// This may generally execute 29+ queries.
// TODO: collect asynchronous calls in a function invoked directly by caller.
void block_chain_impl::fetch_block_locator(block_locator_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, handler](size_t slock)
    {
        hash_list locator;
        size_t top_height;
        if (!database_.blocks.top(top_height))
            return finish_fetch(slock, handler, error::operation_failed,
                locator);

        const auto indexes = block_locator_indexes(top_height);
        for (const auto index: indexes)
        {
            const auto result = database_.blocks.get(index);
            if (!result)
                return finish_fetch(slock, handler, error::not_found, locator);

            locator.push_back(result.header().hash());
        }

        return finish_fetch(slock, handler, error::success, locator);
    };
    fetch_serial(do_fetch);
}

// Fetch start-base-stop|top+1(max 500)
// This may generally execute 502 but as many as 531+ queries.
void block_chain_impl::fetch_locator_block_hashes(
    const message::get_blocks& locator, const hash_digest& threshold,
    size_t limit, locator_block_hashes_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    // This is based on the idea that looking up by block hash to get heights
    // will be much faster than hashing each retrieved block to test for stop.
    const auto do_fetch = [this, locator, threshold, limit, handler](
        size_t slock)
    {
        // Find the first block height.
        // If no start block is on our chain we start with block 0.
        size_t start = 0;
        for (const auto& hash: locator.start_hashes)
        {
            const auto result = database_.blocks.get(hash);
            if (result)
            {
                start = result.height();
                break;
            }
        }

        // Find the stop block height.
        // The maximum stop block is 501 blocks after start (to return 500).
        size_t stop = start + limit + 1;
        if (locator.stop_hash != null_hash)
        {
            // If the stop block is not on chain we treat it as a null stop.
            const auto stop_result = database_.blocks.get(locator.stop_hash);
            if (stop_result)
                stop = std::min(stop_result.height(), stop);
        }

        // Find the threshold block height.
        // If the threshold is above the start it becomes the new start.
        if (threshold != null_hash)
        {
            const auto start_result = database_.blocks.get(threshold);
            if (start_result)
                start = std::max(start_result.height(), start);
        }

        // TODO: This largest portion can be parallelized.
        // Build the hash list until we hit last or the blockchain top.
        hash_list hashes;
        for (size_t index = start + 1; index < stop; ++index)
        {
            const auto result = database_.blocks.get(index);
            if (result)
            {
                hashes.push_back(result.header().hash());
                break;
            }
        }

        return finish_fetch(slock, handler, error::success, hashes);
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_locator_block_headers(
    const message::get_headers& locator, const hash_digest& threshold,
    size_t limit, locator_block_headers_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    // This is based on the idea that looking up by block hash to get heights
    // will be much faster than hashing each retrieved block to test for stop.
    const auto do_fetch = [this, locator, threshold, limit, handler](
        size_t slock)
    {
        // TODO: consolidate this portion with fetch_locator_block_hashes.
        //---------------------------------------------------------------------
        // Find the first block height.
        // If no start block is on our chain we start with block 0.
        size_t start = 0;
        for (const auto& hash: locator.start_hashes)
        {
            const auto result = database_.blocks.get(hash);
            if (result)
            {
                start = result.height();
                break;
            }
        }

        // Find the stop block height.
        // The maximum stop block is 501 blocks after start (to return 500).
        size_t stop = start + limit + 1;
        if (locator.stop_hash != null_hash)
        {
            // If the stop block is not on chain we treat it as a null stop.
            const auto stop_result = database_.blocks.get(locator.stop_hash);
            if (stop_result)
                stop = std::min(stop_result.height(), stop);
        }

        // Find the threshold block height.
        // If the threshold is above the start it becomes the new start.
        if (threshold != null_hash)
        {
            const auto start_result = database_.blocks.get(threshold);
            if (start_result)
                start = std::max(start_result.height(), start);
        }
        //---------------------------------------------------------------------

        // TODO: This largest portion can be parallelized.
        // Build the hash list until we hit last or the blockchain top.
        chain::header::list headers;
        for (size_t index = start + 1; index < stop; ++index)
        {
            const auto result = database_.blocks.get(index);
            if (result)
            {
                headers.push_back(result.header());
                break;
            }
        }

        return finish_fetch(slock, handler, error::success, headers);
    };
    fetch_serial(do_fetch);
}

// This may execute up to 500 queries.
void block_chain_impl::filter_blocks(message::get_data::ptr message,
    result_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped);
        return;
    }

    const auto do_fetch = [this, message, handler](size_t slock)
    {
        auto& inventories = message->inventories;

        for (auto it = inventories.begin(); it != inventories.end();)
            if (it->is_block_type() && database_.blocks.get(it->hash))
                it = inventories.erase(it);
            else
                ++it;

        return finish_fetch(slock, handler, error::success);
    };
    fetch_serial(do_fetch);
}

// BUGBUG: should only remove unspent transactions, other dups ok (BIP30).
void block_chain_impl::filter_transactions(message::get_data::ptr message,
    result_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped);
        return;
    }

    const auto do_fetch = [this, message, handler](size_t slock)
    {
        auto& inventories = message->inventories;

        for (auto it = inventories.begin(); it != inventories.end();)
            if (it->is_transaction_type() &&
                database_.transactions.get(it->hash))
                it = inventories.erase(it);
            else
                ++it;

        return finish_fetch(slock, handler, error::success);
    };
    fetch_serial(do_fetch);
}

/// filter out block hashes that exist in the orphan pool.
void block_chain_impl::filter_orphans(message::get_data::ptr message,
    result_handler handler)
{
    organizer_.filter_orphans(message);
    handler(error::success);
}

// block_chain (formerly fetch_parallel)
// ------------------------------------------------------------------------

void block_chain_impl::fetch_block(uint64_t height,
    block_fetch_handler handler)
{
    blockchain::fetch_block(*this, height, handler);
}

void block_chain_impl::fetch_block(const hash_digest& hash,
    block_fetch_handler handler)
{
    blockchain::fetch_block(*this, hash, handler);
}

void block_chain_impl::fetch_block_header(uint64_t height,
    block_header_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, height, handler](size_t slock)
    {
        const auto result = database_.blocks.get(height);
        return result ?
            finish_fetch(slock, handler, error::success, result.header()) :
            finish_fetch(slock, handler, error::not_found, chain::header());
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_block_header(const hash_digest& hash,
    block_header_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, hash, handler](size_t slock)
    {
        const auto result = database_.blocks.get(hash);
        return result ?
            finish_fetch(slock, handler, error::success, result.header()) :
            finish_fetch(slock, handler, error::not_found, chain::header());
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_merkle_block(uint64_t height,
    merkle_block_fetch_handler handler)
{
    // TODO:
    ////blockchain::fetch_merkle_block(*this, height, handler);
    handler(error::operation_failed, {});
}

void block_chain_impl::fetch_merkle_block(const hash_digest& hash,
    merkle_block_fetch_handler handler)
{
    // TODO:
    ////blockchain::fetch_merkle_block(*this, hash, handler);
    handler(error::operation_failed, {});
}

void block_chain_impl::fetch_block_transaction_hashes(uint64_t height,
    transaction_hashes_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, height, handler](size_t slock)
    {
        const auto result = database_.blocks.get(height);
        return result ?
            finish_fetch(slock, handler, error::success, to_hashes(result)) :
            finish_fetch(slock, handler, error::not_found, hash_list());
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_block_transaction_hashes(const hash_digest& hash,
    transaction_hashes_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, hash, handler](size_t slock)
    {
        const auto result = database_.blocks.get(hash);
        return result ?
            finish_fetch(slock, handler, error::success, to_hashes(result)) :
            finish_fetch(slock, handler, error::not_found, hash_list());
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_block_height(const hash_digest& hash,
    block_height_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, hash, handler](size_t slock)
    {
        const auto result = database_.blocks.get(hash);
        return result ?
            finish_fetch(slock, handler, error::success, result.height()) :
            finish_fetch(slock, handler, error::not_found, 0);
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_last_height(last_height_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, handler](size_t slock)
    {
        size_t last_height;
        return database_.blocks.top(last_height) ?
            finish_fetch(slock, handler, error::success, last_height) :
            finish_fetch(slock, handler, error::not_found, 0);
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_transaction(const hash_digest& hash,
    transaction_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, hash, handler](size_t slock)
    {
        const auto result = database_.transactions.get(hash);
        const auto tx = result ? result.transaction() : chain::transaction();
        return result ?
            finish_fetch(slock, handler, error::success, tx) :
            finish_fetch(slock, handler, error::not_found, tx);
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_transaction_index(const hash_digest& hash,
    transaction_index_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {}, {});
        return;
    }

    const auto do_fetch = [this, hash, handler](size_t slock)
    {
        const auto result = database_.transactions.get(hash);
        return result ?
            finish_fetch(slock, handler, error::success, result.height(),
                result.index()) :
            finish_fetch(slock, handler, error::not_found, 0, 0);
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_spend(const chain::output_point& outpoint,
    spend_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, outpoint, handler](size_t slock)
    {
        const auto spend = database_.spends.get(outpoint);
        const auto point = spend.valid ?
            chain::input_point{ spend.hash, spend.index } :
            chain::input_point();
        return spend.valid ?
            finish_fetch(slock, handler, error::success, point) :
            finish_fetch(slock, handler, error::unspent_output, point);
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_history(const wallet::payment_address& address,
    uint64_t limit, uint64_t from_height, history_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, address, handler, limit, from_height](
        size_t slock)
    {
        const auto history = database_.history.get(address.hash(), limit,
            from_height);
        return finish_fetch(slock, handler, error::success, history);
    };
    fetch_serial(do_fetch);
}

void block_chain_impl::fetch_stealth(const binary& filter, uint64_t from_height,
    stealth_fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto do_fetch = [this, filter, handler, from_height](
        size_t slock)
    {
        const auto stealth = database_.stealth.scan(filter, from_height);
        return finish_fetch(slock, handler, error::success, stealth);
    };
    fetch_serial(do_fetch);
}

} // namespace blockchain
} // namespace libbitcoin
