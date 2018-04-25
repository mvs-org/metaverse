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
#include <metaverse/blockchain/block_chain_impl.hpp>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <unordered_map>
#include <boost/filesystem.hpp>
#include <boost/interprocess/sync/file_lock.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/bitcoin/chain/attachment/asset/asset_cert.hpp>
#include <metaverse/database.hpp>
#include <metaverse/blockchain/block.hpp>
#include <metaverse/blockchain/block_fetcher.hpp>
#include <metaverse/blockchain/organizer.hpp>
#include <metaverse/blockchain/settings.hpp>
#include <metaverse/blockchain/transaction_pool.hpp>
#include <metaverse/blockchain/validate_transaction.hpp>

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
    hash_list hashes;
    hashes.reserve(count);

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

bool block_chain_impl::get_difficulty(u256& out_difficulty,
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

int block_chain_impl::replace_chain(uint64_t begin_height, const block_detail::list& new_blocks, block_detail::list& released_blocks)
{
    int ret = 0;
    bool result = false;

    start_write();
    result = pop_from(released_blocks, begin_height);
    stop_write();
    if(result)
    {
        for (auto it = new_blocks.begin(); ret == 0 && it != new_blocks.end(); ++it)
        {
            auto &block =  *it;

            start_write();
            result = push(block);
            stop_write();
            if(result)
            {
                for(auto& tx : block->actual()->transactions)
                {
                    if (validate_transaction::check_secondaryissue_transaction(tx, *this, false))
                    {
                        ret = it - new_blocks.begin() + 1;
                        block_detail::list blocks;
                        start_write();
                        pop_from(blocks, begin_height);
                        for(auto& b : released_blocks)
                            push(b);
                        stop_write();

                        log::error(LOG_BLOCKCHAIN)
                            << " push block height:" << block->actual()->header.number
                            << " hash:"  << encode_hash(block->actual()->header.hash())
                            << "failed";

                        break;
                    }
                }

                if (ret == 0) {
                    log::debug(LOG_BLOCKCHAIN)
                        << " push block height:" << block->actual()->header.number
                        << " hash:"  << encode_hash(block->actual()->header.hash());
                }
            }
            else
            {
                ret = -1;
                log::error(LOG_BLOCKCHAIN)
                    << " push block height:" << block->actual()->header.number
                    << " hash:"  << encode_hash(block->actual()->header.hash())
                    << "failed";
                break;
            }
        }
    }
    else
    {
        ret = -1;
        log::error(LOG_BLOCKCHAIN)
            << " pop_from begin_height:" << begin_height
            << "failed";
    }

    return ret;
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

    stop_write();
    // Otherwise organize the chain...
    organizer_.organize();

    //...and then get the particular block's status.
    handler(detail->error(), detail->height());
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

    const auto do_read = [try_read]()
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
        	hash_digest hash;
        	auto found = false;
        	{
				const auto result = database_.blocks.get(index);
				if (result)
				{
					found = true;
					hash = result.header().hash();
				}
        	}
			if (!found)
				return finish_fetch(slock, handler, error::not_found, locator);

            locator.push_back(hash);
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
                stop = std::min(stop_result.height() + 1, stop);
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
//                break;
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
                stop = std::min(stop_result.height() + 1, stop);
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
//                break;
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
    	chain::header header;
    	auto found = false;
    	{
    		const auto result = database_.blocks.get(height);
    		if(result)
    		{
    			header = result.header();
    			header.transaction_count = result.transaction_count();
    			found = true;
    		}
    	}
        return found ?
            finish_fetch(slock, handler, error::success, header) :
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
        chain::header header;
		auto found = false;
		{
			const auto result = database_.blocks.get(hash);
			if(result)
			{
				header = result.header();
				header.transaction_count = result.transaction_count();
				found = true;
			}
		}
        return found ?
            finish_fetch(slock, handler, error::success, header) :
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
    	hash_list hashes;
		auto found = false;
		{
			const auto result = database_.blocks.get(height);
			if(result)
			{
				hashes = to_hashes(result);
				found = true;
			}
		}

        return found ?
            finish_fetch(slock, handler, error::success, hashes) :
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
    	hash_list hashes;
		auto found = false;
		{
			const auto result = database_.blocks.get(hash);
			if(result)
			{
				hashes = to_hashes(result);
				found = true;
			}
		}

        return found ?
            finish_fetch(slock, handler, error::success, hashes) :
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
    	std::size_t h{0};
		auto found = false;
		{
			const auto result = database_.blocks.get(hash);
			if(result)
			{
				h = result.height();
				found = true;
			}
		}

        return found ?
            finish_fetch(slock, handler, error::success, h) :
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

bool block_chain_impl::fetch_history(const wallet::payment_address& address,
    uint64_t limit, uint64_t from_height, history_compact::list& history)
{
    if (stopped())
    {
        return false;
    }

    boost::mutex mutex;

    mutex.lock();
    auto f = [&history, &mutex](const code& ec, const history_compact::list& history_) -> void
    {
        if((code)error::success == ec)
            history = history_;
        mutex.unlock();
    };

    // Obtain payment address history from the transaction pool and blockchain.
    fetch_history(address, limit, from_height, f);
    boost::unique_lock<boost::mutex> lock(mutex);

    return true;
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

inline hash_digest block_chain_impl::get_hash(const std::string& str)
{
	data_chunk data(str.begin(), str.end());
	return sha256_hash(data);
}

inline short_hash block_chain_impl::get_short_hash(const std::string& str)
{
	data_chunk data(str.begin(), str.end());
	return ripemd160_hash(data);
}

std::shared_ptr<account> block_chain_impl::is_account_passwd_valid
        (const std::string& name, const std::string& passwd)
{
	auto account = get_account(name);
	if(account && account->get_passwd() == get_hash(passwd)) // account exist
	{
        return account;
	}else{
        throw std::logic_error{"account not found or incorrect password"};
        return nullptr;
    }
}
void block_chain_impl::set_account_passwd
        (const std::string& name, const std::string& passwd)
{
	auto account = get_account(name);
	if(account) // account exist
	{
		account->set_passwd(passwd);
        store_account(account);
	}else{
        throw std::logic_error{"account not found"};
    }
}

bool block_chain_impl::is_admin_account(const std::string& name)
{
	auto account = get_account(name);
	if(account) // account exist
	{
		return account_priority::administrator == account->get_priority();
	}
	return false;
}

bool block_chain_impl::is_account_exist(const std::string& name)
{
	return nullptr != get_account(name);
}
operation_result block_chain_impl::store_account(std::shared_ptr<account> acc)
{
	if (stopped())
	{
		return operation_result::failure;
	}
	if (!(acc))
	{
        throw std::runtime_error{"nullptr for account"};
	}
	///////////////////////////////////////////////////////////////////////////
	// Critical Section.
	unique_lock lock(mutex_);

	const auto hash = get_hash(acc->get_name());
	database_.accounts.store(hash, *acc);
	database_.accounts.sync();
	///////////////////////////////////////////////////////////////////////////
	return operation_result::okay;
}
std::shared_ptr<account> block_chain_impl::get_account(const std::string& name)
{
	return database_.accounts.get_account_result(get_hash(name)).get_account_detail();
}
/// get all the accounts in account database
std::shared_ptr<std::vector<account>> block_chain_impl::get_accounts()
{
	return database_.accounts.get_accounts();
}
/// delete account according account name
operation_result block_chain_impl::delete_account(const std::string& name)
{
	if (stopped())
	{
		return operation_result::failure;
	}
	///////////////////////////////////////////////////////////////////////////
	// Critical Section.
	unique_lock lock(mutex_);

	database_.accounts.remove(get_hash(name));
	database_.accounts.sync();
	///////////////////////////////////////////////////////////////////////////
	return operation_result::okay;
}

/// just store data into database "not do same address check"  -- todo
operation_result block_chain_impl::store_account_address(std::shared_ptr<account_address> address)
{
	if (stopped())
	{
		return operation_result::failure;
	}
	if (!(address))
	{
        throw std::runtime_error{"nullptr for address"};
	}
	///////////////////////////////////////////////////////////////////////////
	// Critical Section.
	unique_lock lock(mutex_);

	const auto hash = get_short_hash(address->get_name());
	database_.account_addresses.store(hash, *address);
	database_.account_addresses.sync();
	///////////////////////////////////////////////////////////////////////////
	return operation_result::okay;
}
/// only delete the last address of account
operation_result block_chain_impl::delete_account_address(const std::string& name)
{
	if (stopped())
	{
		return operation_result::failure;
	}
	///////////////////////////////////////////////////////////////////////////
	// Critical Section.
	unique_lock lock(mutex_);

	auto hash = get_short_hash(name);
	auto addr_vec = database_.account_addresses.get(hash);
	for( auto each : addr_vec )
		database_.account_addresses.delete_last_row(hash);
	database_.account_addresses.sync();
	///////////////////////////////////////////////////////////////////////////
	return operation_result::okay;
}

std::shared_ptr<account_address> block_chain_impl::get_account_address
                    (const std::string& name, const std::string& address)
{
	return database_.account_addresses.get(get_short_hash(name), address);
}

std::shared_ptr<std::vector<account_address>> block_chain_impl::get_account_addresses(const std::string& name)
{
	auto sp_addr = std::make_shared<std::vector<account_address>>();
	auto result = database_.account_addresses.get(get_short_hash(name));
	if(result.size())
	{
		//sp_addr = std::make_shared<std::vector<account_address>>();
	    const auto action = [&sp_addr](const account_address& elem)
	    {
	        sp_addr->emplace_back(std::move(elem)); // todo -- add std::move later
	    };
	    std::for_each(result.begin(), result.end(), action);
	}
	return sp_addr;
}

operation_result block_chain_impl::store_account_asset(const asset_detail& detail)
{
	if (stopped())
	{
		return operation_result::failure;
	}
	///////////////////////////////////////////////////////////////////////////
	// Critical Section.
	unique_lock lock(mutex_);

	const auto hash = get_short_hash(detail.get_issuer());
	database_.account_assets.store(hash, detail);
	database_.account_assets.sync();
	///////////////////////////////////////////////////////////////////////////
	return operation_result::okay;
}

operation_result block_chain_impl::store_account_asset(std::shared_ptr<asset_detail> detail)
{
	if (!(detail))
	{
        throw std::runtime_error{"nullptr for asset"};
	}
	return store_account_asset(*detail);
}

/// delete account asset by account name
operation_result block_chain_impl::delete_account_asset(const std::string& name)
{
	if (stopped())
	{
		return operation_result::failure;
	}
	///////////////////////////////////////////////////////////////////////////
	// Critical Section.
	unique_lock lock(mutex_);

	auto hash = get_short_hash(name);
	auto asset_vec = database_.account_assets.get(hash);
	for( auto each : asset_vec ) // just use asset count
		database_.account_assets.delete_last_row(hash);
	database_.account_assets.sync();
	///////////////////////////////////////////////////////////////////////////
	return operation_result::okay;
}


std::shared_ptr<std::vector<business_address_asset>> block_chain_impl::get_account_asset(const std::string& name,
		const std::string& asset_name, business_kind kind)
{
	auto sp_asset_vec = get_account_assets(name, kind);
	auto ret_vector = std::make_shared<std::vector<business_address_asset>>();

	const auto action = [&](const business_address_asset& addr_asset)
	{
		if(addr_asset.detail.get_symbol() == asset_name)
			ret_vector->emplace_back(std::move(addr_asset));
	};

	std::for_each(sp_asset_vec->begin(), sp_asset_vec->end(), action);

	return ret_vector;
}

std::shared_ptr<std::vector<business_address_asset>> block_chain_impl::get_account_asset(const std::string& name, const std::string& asset_name)
{
	auto sp_asset_vec = get_account_assets(name);
	auto ret_vector = std::make_shared<std::vector<business_address_asset>>();

	const auto action = [&](const business_address_asset& addr_asset)
	{
		if(addr_asset.detail.get_symbol().compare(asset_name) == 0)
			ret_vector->emplace_back(std::move(addr_asset));
	};

	std::for_each(sp_asset_vec->begin(), sp_asset_vec->end(), action);

	return ret_vector;
}

static history::list expand_history(history_compact::list& compact)
{
    history::list result;
    result.reserve(compact.size());

    std::unordered_map<uint64_t, history*> map_output;
    // Process all outputs.
    for (auto output = compact.begin(); output != compact.end(); ++output)
    {
        if (output->kind == point_kind::output)
        {
            history row;
            row.output = output->point;
            row.output_height = output->height;
            row.value = output->value;
            row.spend = { null_hash, max_uint32 };
            row.temporary_checksum = output->point.checksum();
            result.emplace_back(row);
            map_output[row.temporary_checksum] = &result.back();
        }
    }

    //process the spends.
    for (const auto& spend: compact)
    {
        if (spend.kind != point_kind::spend)
            continue;

        auto r = map_output.find(spend.previous_checksum);
        if (r != map_output.end() && r->second->spend.hash == null_hash)
        {
             r->second->spend = spend.point;
             r->second->spend_height = spend.height;
             continue;
        }

        // This will only happen if the history height cutoff comes between
        // an output and its spend. In this case we return just the spend.
        {
            history row;
            row.output = { null_hash, max_uint32 };
            row.output_height = max_uint64;
            row.value = max_uint64;
            row.spend = spend.point;
            row.spend_height = spend.height;
            result.emplace_back(row);
        }
    }

    compact.clear();

    // Clear all remaining checksums from unspent rows.
    for (auto& row: result) {
        if (row.spend.hash == null_hash) {
            row.spend_height = max_uint64;
        }
    }

    // sort by height and index of output, spend or both in order.
    std::sort(result.begin(), result.end(),
        [](const history& elem1, const history& elem2){
            if (elem1.spend_height > elem2.spend_height) { // unspent first, spent time decresely
                return true;
            } else if (elem1.spend_height == elem2.spend_height) {
                if (elem1.output_height < elem2.output_height) { // output time increasely
                    return true;
                } else if (elem1.output_height == elem2.output_height) {
                    return elem1.output.index <= elem2.output.index;
                }
            }
            return false;
        });
    return result;
}

history::list block_chain_impl::get_address_history(wallet::payment_address& addr)
{
    history_compact::list cmp_history;
    if (!fetch_history(addr, 0, 0, cmp_history)) {
        return history::list();
    }
    return expand_history(cmp_history);
}

asset_cert_type block_chain_impl::get_address_asset_cert_type(const std::string& address, const std::string& asset)
{
    BITCOIN_ASSERT(!asset.empty());
    auto business_certs = database_.address_assets.get_asset_certs(address, asset, 0);
    auto certs = asset_cert_ns::none;
    for (const auto& cert : business_certs) {
        certs |= cert.certs.get_certs();
    }
    return certs;
}

std::shared_ptr<std::vector<business_address_asset_cert>>
block_chain_impl::get_address_asset_certs(const std::string& address, const std::string& asset)
{
    auto ret_vector = std::make_shared<std::vector<business_address_asset_cert>>();
    auto business_certs = database_.address_assets.get_asset_certs(address, asset, 0);
    for (auto business_cert : business_certs) {
        ret_vector->emplace_back(std::move(business_cert));
    }
    return ret_vector;
}

std::shared_ptr<std::vector<business_address_asset_cert>>
block_chain_impl::get_account_asset_certs(const std::string& account, const std::string& asset)
{
    auto ret_vector = std::make_shared<std::vector<business_address_asset_cert>>();
    auto pvaddr = get_account_addresses(account);
    if (pvaddr) {
        for (const auto& account_address : *pvaddr) {
            auto business_certs = database_.address_assets.
                get_asset_certs(account_address.get_address(), asset, 0);
            for (auto business_cert : business_certs) {
                ret_vector->emplace_back(std::move(business_cert));
            }
        }
    }
    return ret_vector;
}

std::shared_ptr<std::vector<asset_cert>> block_chain_impl::get_issued_asset_certs()
{
    auto sp_vec = std::make_shared<std::vector<asset_cert>>();
    auto sp_asset_certs_vec = database_.certs.get_blockchain_asset_certs();
    for (const auto& each : *sp_asset_certs_vec)
        sp_vec->emplace_back(std::move(each));
    return sp_vec;
}

bool block_chain_impl::is_asset_cert_exist(const std::string& symbol, asset_cert_type cert_mask)
{
    BITCOIN_ASSERT(!symbol.empty());

    for (int i = 0; i < 64; ++i) {
        const asset_cert_type target_type = (1 << i);
        if (asset_cert::test_certs(cert_mask, target_type)) {
            std::string key_str(symbol + std::to_string(target_type));
            const data_chunk& data = data_chunk(key_str.begin(), key_str.end());
            const auto key = sha256_hash(data);
            auto exist = database_.certs.get(key);
            if (exist == nullptr) {
                return false;
            }
        }
    }

    return true;
}

uint64_t block_chain_impl::get_address_asset_volume(const std::string& addr, const std::string& asset, bool is_use_transactionpool, bool is_safe)
{
	uint64_t asset_volume = 0;
	auto address = payment_address(addr);
    auto&& rows = get_address_history(address);

	chain::transaction tx_temp;
	uint64_t tx_height;

	for (auto& row: rows)
	{
		// spend unconfirmed (or no spend attempted)
		if ((row.spend.hash == null_hash)
				&& get_transaction(tx_temp, tx_height, row.output.hash, is_use_transactionpool, is_safe))
		{
			auto output = tx_temp.outputs.at(row.output.index);

			if((output.is_asset_transfer() || output.is_asset_issue() || output.is_asset_secondaryissue()))
			{
				if(output.get_asset_symbol() == asset)
				{
					asset_volume += output.get_asset_amount();
				}
			}
		}
	}
	return asset_volume;
}

uint64_t block_chain_impl::get_account_asset_volume(const std::string& account, const std::string& asset, bool is_use_transactionpool, bool is_safe)
{
	uint64_t volume = 0;
	auto pvaddr = get_account_addresses(account);

	if(pvaddr)
	{
		for (auto& each : *pvaddr)
		{
			volume += get_address_asset_volume(each.get_address(), asset, is_use_transactionpool, is_safe);
		}
	}

	return volume;
}

uint64_t block_chain_impl::get_asset_volume(const std::string& asset)
{
	return database_.assets.get_asset_volume(asset);
}

// get special assets of the account/name, just used for asset_detail/asset_transfer
std::shared_ptr<std::vector<business_history>> block_chain_impl::get_address_business_history(const std::string& addr,
				const std::string& symbol, business_kind kind, uint8_t confirmed)
{
	auto ret_vector = std::make_shared<std::vector<business_history>>();
	auto sh_vec = database_.address_assets.get_address_business_history(addr, 0);
	std::string asset_symbol;

    for (auto iter = sh_vec->begin(); iter != sh_vec->end(); ++iter){
		if((iter->data.get_kind_value() != kind)
			|| (iter->status != confirmed))
			continue;

		// etp business process
		if (kind == business_kind::etp) {
			ret_vector->emplace_back(std::move(*iter));
			continue;
		}

		// etp award business process
		if (kind == business_kind::etp_award) {
			ret_vector->emplace_back(std::move(*iter));
			continue;
		}

		// asset business process
		asset_symbol = "";
		if(iter->data.get_kind_value() == business_kind::asset_issue) {
			auto transfer = boost::get<asset_detail>(iter->data.get_data());
			asset_symbol = transfer.get_symbol();
		}

		else if(iter->data.get_kind_value() == business_kind::asset_transfer) {
			auto transfer = boost::get<asset_transfer>(iter->data.get_data());
			asset_symbol = transfer.get_symbol();
		}

		else if (iter->data.get_kind_value() == business_kind::asset_cert) {
			auto cert = boost::get<asset_cert>(iter->data.get_data());
			asset_symbol = cert.get_symbol();
		}

        if ( 0 == symbol.compare(asset_symbol)){
            ret_vector->emplace_back(std::move(*iter));
        }
    }

	return ret_vector;
}
// get special assets of the account/name, just used for asset_detail/asset_transfer
std::shared_ptr<std::vector<business_record>> block_chain_impl::get_address_business_record(const std::string& addr,
				uint64_t start, uint64_t end, const std::string& symbol)
{
	auto ret_vector = std::make_shared<std::vector<business_record>>();
	auto sh_vec = database_.address_assets.get(addr, start, end);
	std::string asset_symbol;
	if(symbol.empty()) { // all utxo
	    for (auto iter = sh_vec->begin(); iter != sh_vec->end(); ++iter){
	        ret_vector->emplace_back(std::move(*iter));
	    }
	} else { // asset symbol utxo
	    for (auto iter = sh_vec->begin(); iter != sh_vec->end(); ++iter){
			// asset business process
			asset_symbol = "";
			if(iter->data.get_kind_value() == business_kind::asset_issue) {
				auto transfer = boost::get<asset_detail>(iter->data.get_data());
				asset_symbol = transfer.get_symbol();
			}

			else if(iter->data.get_kind_value() == business_kind::asset_transfer) {
				auto transfer = boost::get<asset_transfer>(iter->data.get_data());
				asset_symbol = transfer.get_symbol();
			}

			else if (iter->data.get_kind_value() == business_kind::asset_cert) {
				auto cert = boost::get<asset_cert>(iter->data.get_data());
				asset_symbol = cert.get_symbol();
			}

	        if (symbol == asset_symbol) {
	            ret_vector->emplace_back(std::move(*iter));
	        }
	    }
	}

	return ret_vector;
}
// get special assets of the account/name, just used for asset_detail/asset_transfer
std::shared_ptr<std::vector<business_record>> block_chain_impl::get_address_business_record(const std::string& address,
    const std::string& symbol, size_t start_height, size_t end_height, uint64_t limit, uint64_t page_number) const
{
	return database_.address_assets.get(address, symbol, start_height, end_height, limit, page_number);
}
// get special assets of the account/name, just used for asset_detail/asset_transfer
std::shared_ptr<std::vector<business_history>> block_chain_impl::get_address_business_history(const std::string& addr,
				business_kind kind, uint8_t confirmed)
{
	auto sp_asset_vec = std::make_shared<std::vector<business_history>>();

	business_history::list asset_vec = database_.address_assets.get_business_history(addr, 0, kind, confirmed);
	const auto add_asset = [&](const business_history& addr_asset)
	{
		sp_asset_vec->emplace_back(std::move(addr_asset));
	};
	std::for_each(asset_vec.begin(), asset_vec.end(), add_asset);

	return sp_asset_vec;
}
// get account owned business history between begin and end
std::shared_ptr<std::vector<business_history>> block_chain_impl::get_account_business_history(const std::string& name,
				business_kind kind, uint32_t time_begin, uint32_t time_end)
{
	auto account_addr_vec = get_account_addresses(name);
	auto sp_asset_vec = std::make_shared<std::vector<business_history>>();

	// copy each asset_vec element to sp_asset
	const auto add_asset = [&](const business_history& addr_asset)
	{
		sp_asset_vec->emplace_back(std::move(addr_asset));
	};

	// search all assets belongs to this address which is owned by account
	const auto action = [&](const account_address& elem)
	{
		auto asset_vec = database_.address_assets.get_business_history(elem.get_address(), 0, kind, time_begin, time_end);
		std::for_each(asset_vec.begin(), asset_vec.end(), add_asset);
	};
	std::for_each(account_addr_vec->begin(), account_addr_vec->end(), action);

	return sp_asset_vec;
}

std::shared_ptr<std::vector<business_history>> block_chain_impl::get_address_business_history(const std::string& addr,
				business_kind kind, uint32_t time_begin, uint32_t time_end)
{
	auto sp_asset_vec = std::make_shared<std::vector<business_history>>();

	business_history::list asset_vec = database_.address_assets.get_business_history(addr, 0, kind, time_begin, time_end);
	const auto add_asset = [&](const business_history& addr_asset)
	{
		sp_asset_vec->emplace_back(std::move(addr_asset));
	};
	std::for_each(asset_vec.begin(), asset_vec.end(), add_asset);

	return sp_asset_vec;
}

std::shared_ptr<std::vector<business_history>> block_chain_impl::get_address_business_history(const std::string& addr)
{
	auto sp_asset_vec = std::make_shared<std::vector<business_history>>();
	auto key = get_short_hash(addr);
	business_history::list asset_vec = database_.address_assets.get_business_history(key, 0);
	const auto add_asset = [&](const business_history& addr_asset)
	{
		sp_asset_vec->emplace_back(std::move(addr_asset));
	};
	std::for_each(asset_vec.begin(), asset_vec.end(), add_asset);

	return sp_asset_vec;
}
/// get business record according address
std::shared_ptr<std::vector<business_record>> block_chain_impl::get_address_business_record(const std::string& addr,
	size_t from_height, size_t limit)
{
	auto sp_asset_vec = std::make_shared<std::vector<business_record>>();
	auto key = get_short_hash(addr);
	business_record::list asset_vec = database_.address_assets.get(key, from_height, limit);
	const auto add_asset = [&](const business_record& addr_asset)
	{
		sp_asset_vec->emplace_back(std::move(addr_asset));
	};
	std::for_each(asset_vec.begin(), asset_vec.end(), add_asset);

	return sp_asset_vec;
}

// get special assets of the account/name, just used for asset_detail/asset_transfer
std::shared_ptr<std::vector<business_address_asset>> block_chain_impl::get_account_assets(const std::string& name,
				business_kind kind)
{
	auto account_addr_vec = get_account_addresses(name);
	auto sp_asset_vec = std::make_shared<std::vector<business_address_asset>>();

	// copy each asset_vec element to sp_asset
	const auto add_asset = [&](const business_address_asset& addr_asset)
	{
		sp_asset_vec->emplace_back(std::move(addr_asset));
	};

	// search all assets belongs to this address which is owned by account
	const auto action = [&](const account_address& elem)
	{
		business_address_asset::list asset_vec = database_.address_assets.get_assets(elem.get_address(), 0, kind);
		std::for_each(asset_vec.begin(), asset_vec.end(), add_asset);
	};
	std::for_each(account_addr_vec->begin(), account_addr_vec->end(), action);

	return sp_asset_vec;
}

// get all assets belongs to the account/name
std::shared_ptr<std::vector<business_address_asset>> block_chain_impl::get_account_assets(const std::string& name)
{
	auto account_addr_vec = get_account_addresses(name);
	auto sp_asset_vec = std::make_shared<std::vector<business_address_asset>>();

	// copy each asset_vec element to sp_asset
	const auto add_asset = [&](const business_address_asset& addr_asset)
	{
		sp_asset_vec->emplace_back(std::move(addr_asset));
	};

	// search all assets belongs to this address which is owned by account
	const auto action = [&](const account_address& elem)
	{
		business_address_asset::list asset_vec = database_.address_assets.get_assets(elem.get_address(), 0);
		std::for_each(asset_vec.begin(), asset_vec.end(), add_asset);
	};
	std::for_each(account_addr_vec->begin(), account_addr_vec->end(), action);

	// get account asset which is not issued (not in blockchain)
	auto no_issued_assets = database_.account_assets.get_unissued_assets(get_short_hash(name));
	std::for_each(no_issued_assets->begin(), no_issued_assets->end(), add_asset);

	return sp_asset_vec;
}

// get all unissued assets which stored in local database
std::shared_ptr<std::vector<business_address_asset>> block_chain_impl::get_account_assets()
{
	auto sh_acc_vec = get_accounts();
	auto ret_vector = std::make_shared<std::vector<business_address_asset>>();

	for(auto& acc : *sh_acc_vec) {
		auto sh_vec = get_account_assets(acc.get_name());
		const auto action = [&](const business_address_asset& addr_asset)
		{
			ret_vector->emplace_back(std::move(addr_asset));
		};
		std::for_each(sh_vec->begin(), sh_vec->end(), action);
	}

	return ret_vector;
}
std::shared_ptr<asset_detail> block_chain_impl::get_account_unissued_asset(const std::string& name,
	const std::string& symbol)
{
	std::shared_ptr<asset_detail> sp_asset(nullptr);
	// copy each asset_vec element to sp_asset
	const auto add_asset = [&](const business_address_asset& addr_asset)
	{
		if(addr_asset.detail.get_symbol() == symbol)
			sp_asset = std::make_shared<asset_detail>(addr_asset.detail);
	};

	// get account asset which is not issued (not in blockchain)
	auto no_issued_assets = database_.account_assets.get_unissued_assets(get_short_hash(name));
	std::for_each(no_issued_assets->begin(), no_issued_assets->end(), add_asset);

	return sp_asset;
}

// get all local unissued assets belongs to the account/name
std::shared_ptr<std::vector<business_address_asset>> block_chain_impl::get_account_unissued_assets(const std::string& name)
{
    auto sp_asset_vec = std::make_shared<std::vector<business_address_asset>>();
    auto sp_issues_asset_vec = get_issued_assets();
    // copy each asset_vec element which is unissued to sp_asset
    const auto add_asset = [&sp_asset_vec, &sp_issues_asset_vec](const business_address_asset& addr_asset)
    {
        auto& symbol = addr_asset.detail.get_symbol();
        auto pos = std::find_if(sp_issues_asset_vec->begin(), sp_issues_asset_vec->end(),
                [&symbol](const asset_detail& elem) { return symbol == elem.get_symbol(); });
        if (pos == sp_issues_asset_vec->end()) { // asset is unissued in blockchain
            sp_asset_vec->emplace_back(std::move(addr_asset));
        }
    };

    // get account asset which is not issued (not in blockchain)
    auto no_issued_assets = database_.account_assets.get_unissued_assets(get_short_hash(name));
    std::for_each(no_issued_assets->begin(), no_issued_assets->end(), add_asset);

    return sp_asset_vec;
}

/// get all the asset in blockchain
std::shared_ptr<std::vector<asset_detail>> block_chain_impl::get_issued_assets()
{
	auto sp_blockchain_vec = database_.assets.get_blockchain_assets();
	auto sp_vec = std::make_shared<std::vector<asset_detail>>();
	for(auto& each : *sp_blockchain_vec)
		sp_vec->push_back(each.get_asset());
	return sp_vec;
}

/* find did symbol exist or not
*  find steps:
*  1. find from blockchain
*  2. find from local database(includes account created asset) if add_local_db = true
*/
bool block_chain_impl::is_did_exist(const std::string& did_name)
{
	// find from blockchain database
	if(get_issued_did(const_cast<std::string&>(did_name)))
		return true;

	return false;
}

/* find did address exist or not
*  find steps:
*  1. find from blockchain
*  2. find from local database(includes account created did) if check_local_db = true
*/
bool block_chain_impl::is_address_issued_did(const std::string& did_address)
{
	// find from blockchain database
	business_address_did::list did_vec = database_.address_dids.get_dids(did_address, 0);

	if (!did_vec.empty())
	{
		return true;
	}

	return false;
}


/* find did by address
*  find steps:
*  1. find from blockchain
*  2. find from local database(includes account created did) if check_local_db = true
*/
std::string block_chain_impl::get_did_from_address(const std::string& did_address)
{
	// find from blockchain database
	business_address_did::list did_vec = database_.address_dids.get_dids(did_address, 0);

	if (!did_vec.empty())
	{
		return did_vec[0].detail.get_symbol();
	}

	return "";
}

/* find history addresses by the did symbol
*/
std::shared_ptr<std::vector<blockchain_did>> block_chain_impl::get_did_history_addresses(const std::string &symbol)
{
    const auto hash = get_hash(symbol);
    return database_.dids.get_history_dids(hash);
}

std::shared_ptr<did_detail> block_chain_impl::get_issued_did(std::string& symbol)
{
	std::shared_ptr<did_detail> sp_did(nullptr);
	const auto hash = get_hash(symbol);
	auto sh_block_did = database_.dids.get(hash);
	if(sh_block_did)
		sp_did = std::make_shared<did_detail>(sh_block_did->get_did());
	return sp_did;
}

/// get all the did in blockchain
std::shared_ptr<std::vector<did_detail>> block_chain_impl::get_issued_dids()
{
    auto sp_vec = std::make_shared<std::vector<did_detail>>();
    if (!sp_vec)
        return nullptr;

    auto sp_blockchain_vec = database_.dids.get_blockchain_dids();
    for (const auto &each : *sp_blockchain_vec){
        if (each.get_status() == blockchain_did::address_current){
            sp_vec->emplace_back(each.get_did());
        }
    }

    return sp_vec;
}

std::shared_ptr<asset_detail> block_chain_impl::get_issued_asset(const std::string& symbol)
{
	std::shared_ptr<asset_detail> sp_asset(nullptr);
	const auto hash = get_hash(symbol);
	auto sh_block_asset = database_.assets.get(hash);
	if(sh_block_asset)
		sp_asset = std::make_shared<asset_detail>(sh_block_asset->get_asset());
	return sp_asset;
}

// get all addresses
std::shared_ptr<std::vector<account_address>> block_chain_impl::get_addresses()
{
	auto sh_acc_vec = get_accounts();
	auto ret_vector = std::make_shared<std::vector<account_address>>();

	for(auto& acc : *sh_acc_vec) {
		auto sh_vec = get_account_addresses(acc.get_name());
		const auto action = [&](const account_address& addr)
		{
			ret_vector->emplace_back(std::move(addr));
		};
		std::for_each(sh_vec->begin(), sh_vec->end(), action);
	}

	return ret_vector;
}

std::shared_ptr<std::vector<business_address_message>> block_chain_impl::get_account_messages(const std::string& name)
{
	auto account_addr_vec = get_account_addresses(name);
	auto sp_asset_vec = std::make_shared<std::vector<business_address_message>>();

	// copy each asset_vec element to sp_asset
	const auto add_asset = [&](const business_address_message& addr_asset)
	{
		sp_asset_vec->emplace_back(std::move(addr_asset));
	};

	// search all assets belongs to this address which is owned by account
	const auto action = [&](const account_address& elem)
	{
		business_address_message::list asset_vec = database_.address_assets.get_messages(elem.get_address(), 0);
		std::for_each(asset_vec.begin(), asset_vec.end(), add_asset);
	};
	std::for_each(account_addr_vec->begin(), account_addr_vec->end(), action);

	return sp_asset_vec;
}

account_status block_chain_impl::get_account_user_status(const std::string& name)
{
	account_status ret_val = account_status::error;
	auto account = get_account(name);
	if(account) // account exist
		ret_val = static_cast<account_status>(account->get_user_status());
	return ret_val;
}
account_status block_chain_impl::get_account_system_status(const std::string& name)
{
	account_status ret_val = account_status::error;
	auto account = get_account(name);
	if(account) // account exist
		ret_val = static_cast<account_status>(account->get_system_status());
	return ret_val;
}

bool block_chain_impl::set_account_user_status(const std::string& name, uint8_t status)
{
	bool ret_val = false;
	auto account = get_account(name);
	if(account) // account exist
	{
		account->set_user_status(status);
		ret_val = true;
	}
	return ret_val;
}

bool block_chain_impl::set_account_system_status(const std::string& name, uint8_t status)
{
	bool ret_val = false;
	auto account = get_account(name);
	if(account) // account exist
	{
		account->set_system_status(status);
		ret_val = true;
	}
	return ret_val;
}

void block_chain_impl::fired()
{
	organizer_.fired();
}
/* find asset symbol exist or not
*  find steps:
*  1. find from blockchain
*  2. find from local database(includes account created asset) if add_local_db = true
*/
bool block_chain_impl::is_asset_exist(const std::string& asset_name, bool add_local_db)
{
	// 1. find from blockchain database
	if(get_issued_asset(const_cast<std::string&>(asset_name)))
		return true;

	// 2. find from local database asset
	if(add_local_db) {
		auto sh_acc_vec = get_local_assets();
		// scan all account asset
		for(auto& acc : *sh_acc_vec) {
			if(asset_name.compare(acc.get_symbol())==0)
				return true;
		}
	}

	return false;
}

bool block_chain_impl::get_asset_height(const std::string& asset_name, uint64_t& height)
{
	const data_chunk& data = data_chunk(asset_name.begin(), asset_name.end());
    const auto hash = sha256_hash(data);

	// std::shared_ptr<blockchain_asset>
	auto sp_asset = database_.assets.get(hash);
	if(sp_asset) {
		height = sp_asset->get_height();
	}

	return (sp_asset != nullptr);
}

bool block_chain_impl::get_did_height(const std::string& did_name, uint64_t& height)
{
	const data_chunk& data = data_chunk(did_name.begin(), did_name.end());
    const auto hash = sha256_hash(data);

	// std::shared_ptr<blockchain_asset>
	auto sp_did = database_.dids.get(hash);
	if(sp_did) {
		height = sp_did->get_height();
	}

	return (sp_did != nullptr);
}
/// get asset from local database including all account's assets
std::shared_ptr<std::vector<asset_detail>> block_chain_impl::get_local_assets()
{
	auto ret_vec = std::make_shared<std::vector<asset_detail>>();
	auto sh_acc_vec = get_accounts();
	// scan all account asset -- maybe the asset has been issued
	for(auto& acc : *sh_acc_vec) {
		auto no_issued_assets = database_.account_assets.get(get_short_hash(acc.get_name()));
		for (auto& detail : no_issued_assets){
			ret_vec->emplace_back(std::move(detail));
		}
	}

	return ret_vec;
}

void block_chain_impl::uppercase_symbol(std::string& symbol)
{
    // uppercase symbol
    for (auto& i : symbol){
        if (!(std::isalnum(i) || i=='.'))
            throw std::logic_error{"symbol must be alpha or number or point."};
        i = std::toupper(i);
    }
}

bool block_chain_impl::is_valid_address(const std::string& address)
{
    return  is_payment_address(address) ||
            is_script_address(address) ||
            is_stealth_address(address) ||
            is_blackhole_address(address);
}

bool block_chain_impl::is_blackhole_address(const std::string& address)
{
	return (address == wallet::payment_address::blackhole_address);
}

bool block_chain_impl::is_stealth_address(const std::string& address)
{
	wallet::stealth_address addr{address};
	return (addr && (addr.version() == wallet::stealth_address::mainnet_p2kh));
}

bool block_chain_impl::is_payment_address(const std::string& address)
{
	wallet::payment_address addr{address};
	return (addr && (addr.version() == wallet::payment_address::mainnet_p2kh));
}

// stupid name for this function. should be is_p2sh_address.
bool block_chain_impl::is_script_address(const std::string& address)
{
	wallet::payment_address addr{address};
	return (addr && (addr.version() == wallet::payment_address::mainnet_p2sh));
}

organizer& block_chain_impl::get_organizer()
{
    return organizer_;
}

bool block_chain_impl::get_transaction(const hash_digest& hash,
    chain::transaction& tx, uint64_t& tx_height)
{

	bool ret = false;
	if (stopped())
    {
        //handler(error::service_stopped, {});
        return ret;
    }

    const auto result = database_.transactions.get(hash);
	if(result) {
		tx = result.transaction();
		tx_height = result.height();
		ret = true;
	} else {
		boost::mutex mutex;
		transaction_message::ptr tx_ptr = nullptr;

		mutex.lock();
		auto f = [&tx_ptr, &mutex](const code& ec, transaction_message::ptr tx_) -> void
		{
			if((code)error::success == ec)
				tx_ptr = tx_;
			mutex.unlock();
		};

		pool().fetch(hash, f);
		boost::unique_lock<boost::mutex> lock(mutex);
		if(tx_ptr) {
			tx = *(static_cast<std::shared_ptr<chain::transaction>>(tx_ptr));
			tx_height = 0;
			ret = true;
		}
	}
	#ifdef MVS_DEBUG
	log::debug("get_transaction=")<<tx.to_string(1);
	#endif

	return ret;

}

bool block_chain_impl::get_transaction_callback(const hash_digest& hash,
    std::function<void(const code&, const chain::transaction&)> handler)
{

	bool ret = false;
	if (stopped())
    {
        //handler(error::service_stopped, {});
        return ret;
    }

    const auto result = database_.transactions.get(hash);
	if(result) {
		handler(error::success, result.transaction());
		ret = true;
	} else {
		transaction_message::ptr tx_ptr = nullptr;

		auto f = [&tx_ptr, handler](const code& ec, transaction_message::ptr tx_) -> void
		{
			if((code)error::success == ec){
				tx_ptr = tx_;
				if(tx_ptr)
					handler(ec, *(static_cast<std::shared_ptr<chain::transaction>>(tx_ptr)));
			}
		};

		pool().fetch(hash, f);
		if(tx_ptr) {
			ret = true;
		}
	}

	return ret;

}

bool block_chain_impl::get_transaction(chain::transaction& tx, uint64_t& tx_height, const hash_digest& hash, bool is_use_transactionpool, bool is_safe)
{
	bool ret = false;
	if (stopped())
    {
        return ret;
    }

    const auto result = database_.transactions.get(hash);
	if(result)
	{
		tx = result.transaction();
		tx_height = result.height();
		ret = true;
	}
	else if(is_use_transactionpool)
	{
		boost::mutex mutex;
		transaction_message::ptr tx_ptr = nullptr;

		mutex.lock();
		auto f = [&tx_ptr, &mutex](const code& ec, transaction_message::ptr tx_) -> void
		{
			if((code)error::success == ec)
				tx_ptr = tx_;
			mutex.unlock();
		};

		if(is_safe)
		{
			pool().fetch(hash, f);
			boost::unique_lock<boost::mutex> lock(mutex);
		}
		else
		{
			pool().sync_fetch(hash, f);
		}

		if(tx_ptr)
		{
			tx = *(static_cast<std::shared_ptr<chain::transaction>>(tx_ptr));
			tx_height = 0;
			ret = true;
		}
	}

	#ifdef MVS_DEBUG
	log::debug("get_transaction=")<<tx.to_string(1);
	#endif

	return ret;
}

bool block_chain_impl::get_history_callback(const payment_address& address,
    size_t limit, size_t from_height,
    std::function<void(const code&, chain::history::list&)> handler)
{

	bool ret = false;
	if (stopped())
    {
        //handler(error::service_stopped, {});
        return ret;
    }

	auto f = [&ret, handler](const code& ec, chain::history_compact::list compact) -> void
	{
		if((code)error::success == ec){
		    history::list result;

		    // Process and remove all outputs.
		    for (auto output = compact.begin(); output != compact.end();)
		    {
		        if (output->kind == point_kind::output)
		        {
		            history row;
		            row.output = output->point;
		            row.output_height = output->height;
		            row.value = output->value;
		            row.spend = { null_hash, max_uint32 };
		            row.temporary_checksum = output->point.checksum();
		            result.emplace_back(row);
		            output = compact.erase(output);
		            continue;
		        }

		        ++output;
		    }

		    // All outputs have been removed, process the spends.
		    for (const auto& spend: compact)
		    {
		        auto found = false;

		        // Update outputs with the corresponding spends.
		        for (auto& row: result)
		        {
		            if (row.temporary_checksum == spend.previous_checksum &&
		                row.spend.hash == null_hash)
		            {
		                row.spend = spend.point;
		                row.spend_height = spend.height;
		                found = true;
		                break;
		            }
		        }

		        // This will only happen if the history height cutoff comes between
		        // an output and its spend. In this case we return just the spend.
		        if (!found)
		        {
		            history row;
		            row.output = { null_hash, max_uint32 };
		            row.output_height = max_uint64;
		            row.value = max_uint64;
		            row.spend = spend.point;
		            row.spend_height = spend.height;
		            result.emplace_back(row);
		        }
		    }

		    compact.clear();

		    // Clear all remaining checksums from unspent rows.
		    for (auto& row: result)
		        if (row.spend.hash == null_hash)
		            row.spend_height = max_uint64;

		    // TODO: sort by height and index of output, spend or both in order.
			handler(ec, result);
			ret = true;
		}
	};

	pool().fetch_history(address, limit, from_height, f);

	return ret;

}

code block_chain_impl::validate_transaction(const chain::transaction& tx)
{

	code ret = error::success;
	if (stopped())
    {
        //handler(error::service_stopped, {});
		log::debug("validate_transaction") << "ec=error::service_stopped";
		ret = error::service_stopped;
        return ret;
    }

	//std::shared_ptr<transaction_message>
	auto tx_ptr = std::make_shared<transaction_message>(tx);
	boost::mutex mutex;

	mutex.lock();
	auto f = [&ret, &mutex](const code& ec, transaction_message::ptr tx_, chain::point::indexes idx_vec) -> void
	{
		log::debug("validate_transaction") << "ec=" << ec << " idx_vec=" << idx_vec.size();
		log::debug("validate_transaction") << "ec.message=" << ec.message();
		//if((error::success == ec) && idx_vec.empty())
		ret = ec;
		mutex.unlock();
	};

	pool().validate(tx_ptr, f);
	boost::unique_lock<boost::mutex> lock(mutex);

	return ret;

}

code block_chain_impl::broadcast_transaction(const chain::transaction& tx)
{

	code ret = error::success;
	if (stopped())
	{
		//handler(error::service_stopped, {});
		log::debug("broadcast_transaction") << "ec=error::service_stopped";
		ret = error::service_stopped;
		return ret;
	}

	//std::shared_ptr<transaction_message>
	using transaction_ptr = std::shared_ptr<transaction_message>;
	auto tx_ptr = std::make_shared<transaction_message>(tx);
	boost::mutex valid_mutex;

	valid_mutex.lock();
	//send_mutex.lock();

    pool().store(tx_ptr, [tx_ptr](const code& ec, transaction_ptr){
		//send_mutex.unlock();
		//ret = true;
    	log::trace("broadcast_transaction") << encode_hash(tx_ptr->hash()) << " confirmed";
    }, [&valid_mutex, &ret, tx_ptr](const code& ec, std::shared_ptr<transaction_message>, chain::point::indexes idx_vec){
		log::debug("broadcast_transaction") << "ec=" << ec << " idx_vec=" << idx_vec.size();
		log::debug("broadcast_transaction") << "ec.message=" << ec.message();
		ret = ec;
		if(error::success == ec){
    		log::trace("broadcast_transaction") << encode_hash(tx_ptr->hash()) << " validated";
		} else {
			//send_mutex.unlock(); // incase dead lock
    		log::trace("broadcast_transaction") << encode_hash(tx_ptr->hash()) << " invalidated";
		}
		valid_mutex.unlock();
    });
	boost::unique_lock<boost::mutex> lock(valid_mutex);
	//boost::unique_lock<boost::mutex> send_lock(send_mutex);

	return ret;

}
bool block_chain_impl::get_history(const wallet::payment_address& address,
    uint64_t limit, uint64_t from_height, history_compact::list& history)
{
	if (stopped())
    {
        //handler(error::service_stopped, {});
        return false;
    }

	boost::mutex mutex;

	mutex.lock();
	auto f = [&history, &mutex](const code& ec, const history_compact::list& history_) -> void
	{
		if((code)error::success == ec)
			history = history_;
		mutex.unlock();
	};

	// Obtain payment address history from the transaction pool and blockchain.
    pool().fetch_history(address, limit, from_height, f);
	boost::unique_lock<boost::mutex> lock(mutex);

#ifdef MVS_DEBUG
	log::debug("get_history=")<<history.size();
#endif

	return true;

}

bool block_chain_impl::get_tx_inputs_etp_value (chain::transaction& tx, uint64_t& etp_val)
{
    chain::transaction tx_temp;
    uint64_t tx_height;
    etp_val = 0;

    for (auto& each : tx.inputs) {

        if (get_transaction(each.previous_output.hash, tx_temp, tx_height)) {
            auto output = tx_temp.outputs.at(each.previous_output.index);
            etp_val += output.value;
        } else {
            log::debug("get_tx_inputs_etp_value=")<<each.to_string(true);
            return false;
        }

    }

    return true;

}

void block_chain_impl::safe_store_account(account& acc, std::vector<std::shared_ptr<account_address>>& addresses)
{
    if (stopped())
        return;

    for(auto& address:addresses) {
        const auto hash = get_short_hash(address->get_name());
        database_.account_addresses.safe_store(hash, *address);
    }

    const auto hash = get_hash(acc.get_name());
    database_.accounts.store(hash, acc);
    database_.account_addresses.sync();
    database_.accounts.sync();

}



} // namespace blockchain
} // namespace libbitcoin
