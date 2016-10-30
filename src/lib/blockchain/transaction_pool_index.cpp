/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin-node.
 *
 * libbitcoin-node is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <bitcoin/blockchain/transaction_pool_index.hpp>

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/blockchain/block.hpp>
#include <bitcoin/blockchain/block_chain.hpp>

namespace libbitcoin {
namespace blockchain {

#define NAME "index"
    
using namespace bc::blockchain;
using namespace bc::chain;
using namespace bc::wallet;
using namespace std::placeholders;

static constexpr uint64_t genesis_height = 0;

transaction_pool_index::transaction_pool_index(threadpool& pool,
    block_chain& blockchain)
  : stopped_(true),
    dispatch_(pool, NAME),
    blockchain_(blockchain)
{
}

// Start and stop.
// ----------------------------------------------------------------------------

void transaction_pool_index::start()
{
    stopped_ = false;
}

void transaction_pool_index::stop()
{
    stopped_ = true;
}

// Utility templates.
// ----------------------------------------------------------------------------

template <typename Point, typename Multimap>
void erase(const payment_address& key, const Point& value_point, Multimap& map)
{
    auto match = [&value_point](const typename Multimap::value_type& entry)
    {
        return entry.second.point == value_point;
    };

    const auto range = map.equal_range(key);
    const auto it = std::find_if(range.first, range.second, match);
    
    if (it != range.second)
        map.erase(it);
}

template <typename InfoList, typename Multimap>
InfoList to_info_list(const payment_address& address, Multimap& map)
{
    auto convert = [](const typename Multimap::value_type& entry)
    {
        return entry.second;
    };

    InfoList out;
    const auto range = map.equal_range(address);
    out.resize(std::distance(range.first, range.second));
    std::transform(range.first, range.second, out.begin(), convert);
    return out;
}

// Add sequence.
// ----------------------------------------------------------------------------

void transaction_pool_index::add(const transaction& tx,
    completion_handler handler)
{
    dispatch_.ordered(
        std::bind(&transaction_pool_index::do_add,
            this, tx, handler));
}

void transaction_pool_index::do_add(const transaction& tx,
    completion_handler handler)
{
    uint32_t index = 0;
    const auto tx_hash = tx.hash();

    for (const auto& input: tx.inputs)
    {
        const auto address = payment_address::extract(input.script);

        if (address)
        {
            const input_point point{ tx_hash, index };
            const spend_info info{ point, input.previous_output };
            spends_map_.emplace(std::move(address), std::move(info));
        }

        ++index;
    }

    index = 0;

    for (const auto& output: tx.outputs)
    {
        const auto address = payment_address::extract(output.script);

        if (address)
        {
            const output_point point{ tx_hash, index };
            const output_info info{ point, output.value };
            outputs_map_.emplace(std::move(address), std::move(info));
        }

        ++index;
    }

    // This is the end of the add sequence.
    handler(error::success);
}

// Remove sequence.
// ----------------------------------------------------------------------------

void transaction_pool_index::remove(const transaction& tx,
    completion_handler handler)
{
    dispatch_.ordered(
        std::bind(&transaction_pool_index::do_remove,
            this, tx, handler));
}

void transaction_pool_index::do_remove(const transaction& tx,
    completion_handler handler)
{
    uint32_t index = 0;
    const auto tx_hash = tx.hash();

    for (const auto& input: tx.inputs)
    {
        const auto address = payment_address::extract(input.script);

        if (address)
            erase(address, input_point{ tx_hash, index }, spends_map_);

        ++index;
    }

    index = 0;

    for (const auto& output: tx.outputs)
    {
        const auto address = payment_address::extract(output.script);

        if (address)
            erase(address, output_point{ tx_hash, index }, outputs_map_);

        ++index;
    }

    // This is the end of the remove sequence.
    handler(error::success);
}

// Fetch all history sequence.
// ----------------------------------------------------------------------------

// Fetch the history first from the blockchain and then from the tx pool index.
void transaction_pool_index::fetch_all_history(const payment_address& address,
    size_t limit, size_t from_height, fetch_handler handler)
{
    blockchain_.fetch_history(address, limit, from_height,
        std::bind(&transaction_pool_index::blockchain_history_fetched,
            this, _1, _2, address, handler));
}

void transaction_pool_index::blockchain_history_fetched(const code& ec,
    const history_list& history, const payment_address& address,
    fetch_handler handler)
{
    if (ec)
    {
        handler(ec, {});
        return;
    }

    fetch_index_history(address,
        std::bind(&transaction_pool_index::index_history_fetched,
            _1, _2, _3, history, handler));
}

void transaction_pool_index::index_history_fetched(const code& ec,
    const spend_info::list& spends, const output_info::list& outputs,
    const history_list& history, fetch_handler handler)
{
    if (ec)
    {
        handler(ec, {});
        return;
    }

    // Copy the list for modification and return.
    auto out = history;

    // Race conditions raise the possiblity of seeing a spend or output more
    // than once. We collapse any duplicates here and continue.
    add(out, spends);
    add(out, outputs);

    // This is the end of the fetch_all_history sequence.
    handler(error::success, out);
}

// Fetch index history sequence.
// ----------------------------------------------------------------------------

// Fetch history from the transaction pool index only.
void transaction_pool_index::fetch_index_history(
    const payment_address& address, query_handler handler)
{
    dispatch_.ordered(
        std::bind(&transaction_pool_index::do_fetch,
            this, address, handler));
}

void transaction_pool_index::do_fetch(const payment_address& address,
    query_handler handler)
{
    // This is the end of the fetch_index_history sequence.
    handler(error::success,
        to_info_list<spend_info::list>(address, spends_map_),
        to_info_list<output_info::list>(address, outputs_map_));
}

// Static helpers
// ----------------------------------------------------------------------------
// Transactions may exist in the memory pool and in the blockchain,
// although this circumstance should not persist.

bool transaction_pool_index::exists(history_list& history,
    const spend_info& spend)
{
    const auto match = [&spend](const history_compact& row)
    {
        return row.kind == point_kind::spend && row.point == spend.point;
    };

    return std::any_of(history.begin(), history.end(), match);
}

bool transaction_pool_index::exists(history_list& history,
    const output_info& output)
{
    const auto match = [&output](const history_compact& row)
    {
        return row.kind == point_kind::output && row.point == output.point;
    };

    return std::any_of(history.begin(), history.end(), match);
}

void transaction_pool_index::add(history_list& history, const spend_info& spend)
{
    const history_compact row
    {
        point_kind::spend,
        spend.point,
        genesis_height,
        { spend.previous_output.checksum() }
    };

    history.emplace_back(std::move(row));
}

void transaction_pool_index::add(history_list& history, const output_info& output)
{
    const history_compact row
    {
        point_kind::output,
        output.point,
        genesis_height,
        { output.value }
    };

    history.emplace_back(std::move(row));
}

void transaction_pool_index::add(history_list& history,
    const spend_info::list& spends)
{
    const auto action = [&history](const spend_info& spend)
    {
        if (!exists(history, spend))
            add(history, spend);
    };

    std::for_each(spends.begin(), spends.end(), action);
}

void transaction_pool_index::add(history_list& history,
    const output_info::list& outputs)
{
    const auto action = [&history](const output_info& output)
    {
        if (!exists(history, output))
            add(history, output);
    };

    std::for_each(outputs.begin(), outputs.end(), action);
}

} // namespace blockchain
} // namespace libbitcoin
