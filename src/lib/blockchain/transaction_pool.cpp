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
#include <bitcoin/blockchain/transaction_pool.hpp>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <system_error>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/blockchain/block_chain.hpp>
#include <bitcoin/blockchain/settings.hpp>
#include <bitcoin/blockchain/validate_transaction.hpp>

namespace libbitcoin {
namespace blockchain {

#define NAME "mempool"

using namespace chain;
using namespace wallet;
using namespace std::placeholders;

transaction_pool::transaction_pool(threadpool& pool, block_chain& chain,
    const settings& settings)
  : stopped_(true),
    maintain_consistency_(settings.transaction_pool_consistency),
    buffer_(settings.transaction_pool_capacity),
    dispatch_(pool, NAME),
    blockchain_(chain),
    index_(pool, chain),
    subscriber_(std::make_shared<transaction_subscriber>(pool, NAME))
{
}

transaction_pool::~transaction_pool()
{
    clear(error::service_stopped);
}

void transaction_pool::start()
{
    stopped_ = false;
    index_.start();
    subscriber_->start();

    // Subscribe to blockchain (organizer) reorg notifications.
    blockchain_.subscribe_reorganize(
        std::bind(&transaction_pool::handle_reorganized,
            this, _1, _2, _3, _4));
}

// The subscriber is not restartable.
void transaction_pool::stop()
{
    stopped_ = true;
    index_.stop();
    subscriber_->stop();
    subscriber_->invoke(error::service_stopped, {}, {});
}

bool transaction_pool::stopped()
{
    return stopped_;
}

void transaction_pool::inventory(message::inventory::ptr inventory)
{
    ///////////////////////////////////////////////////////////////////////////
    // TODO: populate the inventory vector from the full memory pool.
    ///////////////////////////////////////////////////////////////////////////
}

void transaction_pool::validate(transaction_ptr tx, validate_handler handler)
{
    dispatch_.ordered(&transaction_pool::do_validate,
        this, tx, handler);
}

void transaction_pool::do_validate(transaction_ptr tx,
    validate_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, tx, {});
        return;
    }

    const auto validate = std::make_shared<validate_transaction>(
        blockchain_, *tx, *this, dispatch_);

    validate->start(
        dispatch_.ordered_delegate(&transaction_pool::handle_validated,
            this, _1, _2, _3, handler));
}

void transaction_pool::handle_validated(const code& ec, transaction_ptr tx,
    const indexes& unconfirmed, validate_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, tx, {});
        return;
    }

    if (ec == error::input_not_found || ec == error::validate_inputs_failed)
    {
        BITCOIN_ASSERT(unconfirmed.size() == 1);
        handler(ec, tx, unconfirmed);
        return;
    }

    if (ec)
    {
        BITCOIN_ASSERT(unconfirmed.empty());
        handler(ec, tx, {});
        return;
    }

    // Recheck the memory pool, as a duplicate may have been added.
    if (is_in_pool(tx->hash()))
        handler(error::duplicate, tx, {});
    else
        handler(error::success, tx, unconfirmed);
}

// handle_confirm will never fire if handle_validate returns a failure code.
void transaction_pool::store(transaction_ptr tx,
    confirm_handler handle_confirm, validate_handler handle_validate)
{
    if (stopped())
    {
        handle_validate(error::service_stopped, tx, {});
        return;
    }

    validate(tx,
        std::bind(&transaction_pool::do_store,
            this, _1, _2, _3, handle_confirm, handle_validate));
}

// This is overly complex due to the transaction pool and index split.
void transaction_pool::do_store(const code& ec, transaction_ptr tx,
    const indexes& unconfirmed, confirm_handler handle_confirm,
    validate_handler handle_validate)
{
    if (ec)
    {
        handle_validate(ec, tx, {});
        return;
    }

    // Set up deindexing to run after transaction pool removal.
    const auto do_deindex = [this, handle_confirm](const code ec,
        transaction_ptr tx)
    {
        const auto do_confirm = [handle_confirm, tx, ec](const code)
        {
            handle_confirm(ec, tx);
        };

        // This always sets success but we have captured the confirmation code.
        index_.remove(*tx, do_confirm);
    };

    // Add to pool, save confirmation handler.
    add(tx, do_deindex);

    const auto handle_indexed = [this, handle_validate, tx, unconfirmed](
        const code ec)
    {
        // Notify subscribers that the tx has been validated and indexed.
        notify_transaction(unconfirmed, tx);

        log::debug(LOG_BLOCKCHAIN)
            << "Transaction saved to mempool (" << buffer_.size() << ")";

        // Notify caller that the tx has been validated and indexed.
        handle_validate(ec, tx, unconfirmed);
    };

    // Add to index and invoke handler to indicate validation and indexing.
    index_.add(*tx, handle_indexed);
}

void transaction_pool::fetch(const hash_digest& transaction_hash,
    fetch_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped, {});
        return;
    }

    const auto tx_fetcher = [this, transaction_hash, handler]()
    {
        const auto it = find(transaction_hash);

        if (it == buffer_.end())
            handler(error::not_found, {});
        else
            handler(error::success, it->tx);
    };

    dispatch_.ordered(tx_fetcher);
}

void transaction_pool::fetch_history(const payment_address& address,
    size_t limit, size_t from_height,
    block_chain::history_fetch_handler handler)
{
    // This passes through to blockchain to build combined history.
    index_.fetch_all_history(address, limit, from_height, handler);
}

// TODO: use hash table pool to eliminate this O(n^2) search.
void transaction_pool::filter(get_data_ptr message, result_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped);
        return;
    }

    const auto filter_transactions = [this, message, handler]()
    {
        auto& inventories = message->inventories;

        for (auto it = inventories.begin(); it != inventories.end();)
            if (it->is_transaction_type() && is_in_pool(it->hash))
                it = inventories.erase(it);
            else
                ++it;

        handler(error::success);
    };

    dispatch_.ordered(filter_transactions);
}

void transaction_pool::exists(const hash_digest& tx_hash,
    result_handler handler)
{
    if (stopped())
    {
        handler(error::service_stopped);
        return;
    }

    const auto get_existence = [this, tx_hash, handler]()
    {
        handler(is_in_pool(tx_hash) ? error::success : error::not_found);
    };

    dispatch_.ordered(get_existence);
}

// new blocks come in - remove txs in new
// old blocks taken out - resubmit txs in old
bool transaction_pool::handle_reorganized(const code& ec, size_t fork_point,
    const block_list& new_blocks, const block_list& replaced_blocks)
{
    if (ec == error::service_stopped)
    {
        log::debug(LOG_BLOCKCHAIN)
            << "Stopping transaction pool: " << ec.message();
        return false;
    }

    if (ec)
    {
        log::debug(LOG_BLOCKCHAIN)
            << "Failure in tx pool reorganize handler: " << ec.message();
        return false;
    }

    log::debug(LOG_BLOCKCHAIN)
        << "Reorganize: tx pool size (" << buffer_.size()
        << ") forked at (" << fork_point
        << ") new blocks (" << new_blocks.size()
        << ") replace blocks (" << replaced_blocks.size() << ")";

    if (replaced_blocks.empty())
    {
        // Remove memory pool transactions that also exist in new blocks.
        dispatch_.ordered(
            std::bind(&transaction_pool::remove,
                this, new_blocks));
    }
    else
    {
        // See http://www.jwz.org/doc/worse-is-better.html
        // for why we take this approach. We return with an error_code.
        // An alternative would be resubmit all tx from the cleared blocks.
        dispatch_.ordered(
            std::bind(&transaction_pool::clear,
                this, error::blockchain_reorganized));
    }

    return true;
}

void transaction_pool::subscribe_transaction(
    transaction_handler handle_transaction)
{
    subscriber_->subscribe(handle_transaction, error::service_stopped, {}, {});
}

void transaction_pool::notify_transaction(const point::indexes& unconfirmed,
    transaction_ptr tx)
{
    subscriber_->relay(error::success, unconfirmed, tx);
}

// Entry methods.
// ----------------------------------------------------------------------------

// A new transaction has been received, add it to the memory pool.
void transaction_pool::add(transaction_ptr tx, confirm_handler handler)
{
    // When a new tx is added to the buffer drop the oldest.
    if (maintain_consistency_ && buffer_.size() == buffer_.capacity())
        delete_package(error::pool_filled);

    buffer_.push_back({ tx, handler });
}

// There has been a reorg, clear the memory pool using the given reason code.
void transaction_pool::clear(const code& ec)
{
    for (const auto& entry: buffer_)
        entry.handle_confirm(ec, entry.tx);

    buffer_.clear();
}

// Delete memory pool txs that are obsoleted by a new block acceptance.
void transaction_pool::remove(const block_list& blocks)
{
    // Delete by hash sets a success code.
    delete_confirmed_in_blocks(blocks);

    // Delete by spent sets a double-spend error.
    if (maintain_consistency_)
        delete_spent_in_blocks(blocks);
}

// Consistency methods.
// ----------------------------------------------------------------------------

// Delete mempool txs that are duplicated in the new blocks.
void transaction_pool::delete_confirmed_in_blocks(const block_list& blocks)
{
    if (stopped() || buffer_.empty())
        return;

    for (const auto block: blocks)
        for (const auto& tx: block->transactions)
            delete_single(tx.hash(), error::success);
}

// Delete all txs that spend a previous output of any tx in the new blocks.
void transaction_pool::delete_spent_in_blocks(const block_list& blocks)
{
    if (stopped() || buffer_.empty())
        return;

    for (const auto block: blocks)
        for (const auto& tx: block->transactions)
            for (const auto& input: tx.inputs)
                delete_dependencies(input.previous_output,
                    error::double_spend);
}

// Delete any tx that spends any output of this tx.
void transaction_pool::delete_dependencies(const output_point& point,
    const code& ec)
{
    const auto comparitor = [&point](const input& input)
    {
        return input.previous_output == point;
    };

    delete_dependencies(comparitor, ec);
}

// Delete any tx that spends any output of this tx.
void transaction_pool::delete_dependencies(const hash_digest& tx_hash,
    const code& ec)
{
    const auto comparitor = [&tx_hash](const input& input)
    {
        return input.previous_output.hash == tx_hash;
    };

    delete_dependencies(comparitor, ec);
}

// This is horribly inefficient, but it's simple.
// TODO: Create persistent multi-indexed memory pool (including age and
// children) and perform this pruning trivialy (and add policy over it).
void transaction_pool::delete_dependencies(input_compare is_dependency,
    const code& ec)
{
    std::vector<entry> dependencies;
    for (const auto& entry: buffer_)
        for (const auto& input: entry.tx->inputs)
            if (is_dependency(input))
            {
                dependencies.push_back(entry);
                break;
            }

    // We queue deletion to protect the iterator.
    for (const auto& dependency: dependencies)
        delete_package(dependency.tx, ec);
}

void transaction_pool::delete_package(const code& ec)
{
    if (stopped() || buffer_.empty())
        return;

    // Must copy the entry because it is going to be deleted from the list.
    const auto oldest = buffer_.front();

    oldest.handle_confirm(ec, oldest.tx);
    delete_package(oldest.tx, ec);
}

void transaction_pool::delete_package(transaction_ptr tx, const code& ec)
{
    if (delete_single(tx->hash(), ec))
        delete_dependencies(tx->hash(), ec);
}

bool transaction_pool::delete_single(const hash_digest& tx_hash, const code& ec)
{
    if (stopped())
        return false;

    const auto matched = [&tx_hash](const entry& entry)
    {
        return entry.tx->hash() == tx_hash;
    };

    const auto it = std::find_if(buffer_.begin(), buffer_.end(), matched);

    if (it == buffer_.end())
        return false;

    it->handle_confirm(ec, it->tx);
    buffer_.erase(it);
    return true;
}

bool transaction_pool::find(transaction_ptr& out_tx,
    const hash_digest& tx_hash) const
{
    const auto it = find(tx_hash);
    const auto found = it != buffer_.end();

    if (found)
        out_tx = it->tx;

    return found;
}

bool transaction_pool::find(chain::transaction& out_tx,
    const hash_digest& tx_hash) const
{
    const auto it = find(tx_hash);
    const auto found = it != buffer_.end();

    if (found)
    {
        // TRANSACTION COPY
        out_tx = *(it->tx);
    }

    return found;
}

transaction_pool::const_iterator transaction_pool::find(
    const hash_digest& tx_hash) const
{
    const auto found = [&tx_hash](const entry& entry)
    {
        return entry.tx->hash() == tx_hash;
    };

    return std::find_if(buffer_.begin(), buffer_.end(), found);
}

bool transaction_pool::is_in_pool(const hash_digest& tx_hash) const
{
    return find(tx_hash) != buffer_.end();
}

bool transaction_pool::is_spent_in_pool(transaction_ptr tx) const
{
    return is_spent_in_pool(*tx);
}

bool transaction_pool::is_spent_in_pool(const transaction& tx) const
{
    const auto found = [this](const input& input)
    {
        return is_spent_in_pool(input.previous_output);
    };

    const auto& inputs = tx.inputs;
    return std::any_of(inputs.begin(), inputs.end(), found);
}

bool transaction_pool::is_spent_in_pool(const output_point& outpoint) const
{
    const auto found = [this, &outpoint](const entry& entry)
    {
        return is_spent_by_tx(outpoint, entry.tx);
    };

    return std::any_of(buffer_.begin(), buffer_.end(), found);
}

bool transaction_pool::is_spent_by_tx(const output_point& outpoint,
    transaction_ptr tx)
{
    const auto found = [&outpoint](const input& input)
    {
        return input.previous_output == outpoint;
    };

    const auto& inputs = tx->inputs;
    return std::any_of(inputs.begin(), inputs.end(), found);
}

} // namespace blockchain
} // namespace libbitcoin
