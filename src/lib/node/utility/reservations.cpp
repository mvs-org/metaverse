/**
 * Copyright (c) 2011-2020 libbitcoin developers (see AUTHORS)
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
#include <metaverse/node/utility/reservations.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>
#include <metaverse/bitcoin.hpp>
#include <metaverse/node/utility/header_queue.hpp>
#include <metaverse/node/utility/performance.hpp>
#include <metaverse/node/utility/reservation.hpp>

namespace libbitcoin {
namespace node {

using namespace bc::blockchain;
using namespace bc::chain;

// The protocol maximum size of get data block requests.
static constexpr size_t max_block_request = 50000;

reservations::reservations(header_queue& hashes, simple_chain& chain,
    const settings& settings)
  : hashes_(hashes),
    blockchain_(chain),
    max_request_(max_block_request),
    timeout_(settings.block_timeout_seconds)
{
    initialize(settings.download_connections);
}

bool reservations::import(block::ptr block, size_t height)
{
    // Thread safe.
    return blockchain_.import(block, height);
}

// Rate methods.
//-----------------------------------------------------------------------------

// A statistical summary of block import rates.
// This computation is not synchronized across rows because rates are cached.
reservations::rate_statistics reservations::rates() const
{
    // Copy row pointer table to prevent need for lock during iteration.
    auto rows = table();
    const auto idle = [](reservation::ptr row)
    {
        return row->idle();
    };

    // Remove idle rows from the table.
    rows.erase(std::remove_if(rows.begin(), rows.end(), idle), rows.end());
    const auto active_rows = rows.size();

    std::vector<double> rates(active_rows);
    const auto normal_rate = [](reservation::ptr row)
    {
        return row->rate().normal();
    };

    // Convert to a rates table and sum.
    std::transform(rows.begin(), rows.end(), rates.begin(), normal_rate);
    const auto total = std::accumulate(rates.begin(), rates.end(), 0.0);

    // Calculate mean and sum of deviations.
    const auto mean = divide<double>(total, active_rows);
    const auto summary = [mean](double initial, double rate)
    {
        const auto difference = mean - rate;
        return initial + (difference * difference);
    };

    // Calculate the standard deviation in the rate deviations.
    auto squares = std::accumulate(rates.begin(), rates.end(), 0.0, summary);
    auto quotient = divide<double>(squares, active_rows);
    auto standard_deviation = std::sqrt(quotient);
    return{ active_rows, mean, standard_deviation };
}

// Table methods.
//-----------------------------------------------------------------------------

reservation::list reservations::table() const
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    shared_lock lock(mutex_);

    return table_;
    ///////////////////////////////////////////////////////////////////////////
}

void reservations::remove(reservation::ptr row)
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_upgrade();

    const auto it = std::find(table_.begin(), table_.end(), row);

    if (it == table_.end())
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        return;
    }

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    mutex_.unlock_upgrade_and_lock();

    table_.erase(it);

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////
}

// Hash methods.
//-----------------------------------------------------------------------------

// Mark hashes for blocks we already have.
void reservations::mark_existing()
{
    uint64_t gap;
    auto start = hashes_.first_height();

    // Not thread safe. Returns false when first is > count (last gap).
    while (blockchain_.get_next_gap(gap, start))
    {
        hashes_.invalidate(start, gap - start);
        start = gap + 1;
    }
}

// No critical section because this is private to the constructor.
// TODO: Optimize by modifying allocation loop to evenly dstribute gap
// reservations so that re-population is not required. Alternatively
// convert header_queue to a height/header map and remove instead of mark.
void reservations::initialize(size_t size)
{
    // Guard against overflow by capping size.
    const size_t max_rows = max_size_t / max_request();
    auto rows = std::min(max_rows, size);

    // Ensure that there is at least one block per row.
    const auto blocks = hashes_.size();
    rows = std::min(rows, blocks);

    if (rows == 0)
        return;

    mark_existing();
    table_.reserve(rows);

    // Allocate no more than 50k headers per row.
    const auto max_allocation = rows * max_request();
    const auto allocation = std::min(blocks, max_allocation);

    for (auto row = 0u; row < rows; ++row)
        table_.push_back(std::make_shared<reservation>(*this, row, timeout_));

    size_t count = 0;
    size_t height;
    hash_digest hash;

    // The (allocation / rows) * rows cannot exceed allocation.
    // The remainder is retained by the hash list for later reservation.
    for (size_t base = 0; base < (allocation / rows); ++base)
    {
        for (size_t row = 0; row < rows; ++row)
        {
            hashes_.dequeue(hash, height);

            if (hashes_.valid(hash))
            {
                ++count;
                table_[row]->insert(hash, height);
            }
        }
    }

    // This is required as any rows left empty above will not populate or stop.
    for (auto row: table_)
        row->populate();

    log::debug(LOG_NODE)
        << "Reserved " << count << " blocks to " << rows << " slots.";
}

// Call when minimal is empty.
bool reservations::populate(reservation::ptr minimal)
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock();

    // Take from unallocated or allocated hashes, true if minimal not empty.
    const auto populated = reserve(minimal) || partition(minimal);

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    if (populated)
        log::debug(LOG_NODE)
            << "Populated " << minimal->size() << " blocks to slot ("
            << minimal->slot() << ").";

    return populated;
}

// This can cause reduction of an active reservation.
bool reservations::partition(reservation::ptr minimal)
{
    const auto maximal = find_maximal();
    return maximal && maximal != minimal && maximal->partition(minimal);
}

reservation::ptr reservations::find_maximal()
{
    if (table_.empty())
        return nullptr;

    // The maximal row is that with the most block hashes reserved.
    const auto comparer = [](reservation::ptr left, reservation::ptr right)
    {
        return left->size() < right->size();
    };

    return *std::max_element(table_.begin(), table_.end(), comparer);
}

// Return false if minimal is empty.
bool reservations::reserve(reservation::ptr minimal)
{
    if (!minimal->empty())
        return true;

    const auto allocation = std::min(hashes_.size(), max_request());

    size_t height;
    hash_digest hash;

    for (size_t block = 0; block < allocation; ++block)
    {
        hashes_.dequeue(hash, height);

        if (hashes_.valid(hash))
            minimal->insert(hash, height);
    }

    // This may become empty between insert and this test, which is okay.
    return !minimal->empty();
}

// Exposed for test to be able to control the request size.
size_t reservations::max_request() const
{
    return max_request_.load();
}

// Exposed for test to be able to control the request size.
void reservations::set_max_request(size_t value)
{
    max_request_.store(value);
}

} // namespace node
} // namespace libbitcoin
