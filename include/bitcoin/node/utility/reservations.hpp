/**
 * Copyright (c) 2011-2016 libbitcoin developers (see AUTHORS)
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
#ifndef LIBBITCOIN_NODE_RESERVATIONS_HPP
#define LIBBITCOIN_NODE_RESERVATIONS_HPP

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <bitcoin/blockchain.hpp>
#include <bitcoin/node/define.hpp>
#include <bitcoin/node/settings.hpp>
#include <bitcoin/node/utility/header_queue.hpp>
#include <bitcoin/node/utility/reservation.hpp>

namespace libbitcoin {
namespace node {

// Class to manage a set of reservation objects during sync, thread safe.
class BCN_API reservations
{
public:
    typedef struct
    {
        size_t active_count;
        double arithmentic_mean;
        double standard_deviation;
    } rate_statistics;

    typedef std::shared_ptr<reservations> ptr;

    /// Construct a reservation table of reservations, allocating hashes evenly
    /// among the rows up to the limit of a single get headers p2p request.
    reservations(header_queue& hashes, blockchain::simple_chain& chain,
        const settings& settings);

    /// The average and standard deviation of block import rates.
    rate_statistics rates() const;

    /// Return a copy of the reservation table.
    reservation::list table() const;

    /// Import the given block to the blockchain at the specified height.
    bool import(chain::block::ptr block, size_t height);

    /// Populate a starved row by taking half of the hashes from a weak row.
    bool populate(reservation::ptr minimal);

    /// Remove the row from the reservation table if found.
    void remove(reservation::ptr row);

    /// The max size of a block request.
    size_t max_request() const;

    /// Set the max size of a block request (defaults to 50000).
    void set_max_request(size_t value);

private:

    // Create the specified number of reservations and distribute hashes.
    void initialize(size_t size);

    // Mark hashes for blocks we already have.
    void mark_existing();

    // Find the reservation with the most hashes.
    reservation::ptr find_maximal();

    // Move half of the maximal reservation to the specified reservation.
    bool partition(reservation::ptr minimal);

    // Move the maximum unreserved hashes to the specified reservation.
    bool reserve(reservation::ptr minimal);

    // Thread safe.
    header_queue& hashes_;
    blockchain::simple_chain& blockchain_;

    // Protected by mutex.
    reservation::list table_;
    mutable upgrade_mutex mutex_;

    const uint32_t timeout_;
    std::atomic<size_t> max_request_;
};

} // namespace node
} // namespace libbitcoin

#endif

