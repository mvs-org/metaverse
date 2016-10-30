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
#ifndef LIBBITCOIN_NODE_RESERVATION_HPP
#define LIBBITCOIN_NODE_RESERVATION_HPP

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <bitcoin/blockchain.hpp>
#include <bitcoin/node/define.hpp>
#include <bitcoin/node/utility/performance.hpp>

namespace libbitcoin {
namespace node {

class reservations;

// Class to manage hashes during sync, thread safe.
class BCN_API reservation
  : public enable_shared_from_base<reservation>
{
public:
    typedef std::shared_ptr<reservation> ptr;
    typedef std::vector<reservation::ptr> list;

    /// Construct a block reservation with the specified identifier.
    reservation(reservations& reservations, size_t slot,
        uint32_t block_timeout_seconds);

    /// Ensure there are no remaining reserved hashes.
    ~reservation();

    /// The sequential identifier of this reservation.
    size_t slot() const;

    /// True if there are currently no hashes.
    bool empty() const;

    /// The number of outstanding blocks.
    size_t size() const;

    /// The reservation is empty and will remain so.
    bool stopped() const;

    /// True if block import rate was more than one standard deviation low.
    bool expired() const;

    /// Sets the idle state to true. Call when channel is stopped.
    void reset();

    /// True if the reservation is not applied to a channel.
    bool idle() const;

    /// The current cached average block import rate excluding import time.
    performance rate() const;

    /// The current cached average block import rate excluding import time.
    void set_rate(const performance& rate);

    /// The block data request message for the outstanding block hashes.
    /// Set new if the preceding request was unsuccessful or discarded.
    message::get_data request(bool new_channel);

    /// Add the block hash to the reservation.
    void insert(const config::checkpoint& checkpoint);

    /// Add the block hash to the reservation.
    void insert(const hash_digest& hash, size_t height);

    /// Add to the blockchain, with height determined by the reservation.
    void import(chain::block::ptr block);

    /// Determine if the reservation was partitioned and reset partition flag.
    bool toggle_partitioned();

    /// Move half of the reservation to the specified reservation.
    bool partition(reservation::ptr minimal);

    /// If not stopped and if empty try to get more hashes.
    void populate();

protected:
    // Accessor for testability.
    bool pending() const;

    // Accessor for testability.
    void set_pending(bool value);

    // Accessor for validating construction.
    std::chrono::microseconds rate_window() const;

    // Isolation of side effect to enable unit testing.
    virtual std::chrono::high_resolution_clock::time_point now() const;

private:
    typedef struct
    {
        size_t events;
        uint64_t database;
        std::chrono::high_resolution_clock::time_point time;
    } import_record;

    typedef std::vector<import_record> rate_history;

    // A bidirection map is used for efficient hash and height retrieval.
    typedef boost::bimaps::bimap<
        boost::bimaps::unordered_set_of<hash_digest>,
        boost::bimaps::set_of<uint32_t>> hash_heights;

    // Return rate history to startup state.
    void clear_history();

    // Get the height of the block hash, remove and return true if it is found.
    bool find_height_and_erase(const hash_digest& hash, uint32_t& out_height);

    // Update rate history to reflect an additional block of the given size.
    void update_rate(size_t events, const std::chrono::microseconds& database);

    // Thread safe.
    reservations& reservations_;

    // Protected by rate mutex.
    performance rate_;
    mutable upgrade_mutex rate_mutex_;

    // Protected by history mutex.
    rate_history history_;
    mutable upgrade_mutex history_mutex_;

    // Protected by stop mutex.
    bool stopped_;
    mutable upgrade_mutex stop_mutex_;

    // Protected by hash mutex.
    bool pending_;
    bool partitioned_;
    hash_heights heights_;
    mutable upgrade_mutex hash_mutex_;

    const size_t slot_;
    const std::chrono::microseconds rate_window_;
};

} // namespace node
} // namespace libbitcoin

#endif

