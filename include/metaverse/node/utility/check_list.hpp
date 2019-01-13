/**
 * Copyright (c) 2011-2017 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MVS_NODE_CHECK_LIST_HPP
#define MVS_NODE_CHECK_LIST_HPP

#include <cstddef>
#include <boost/bimap.hpp>
#include <boost/bimap/set_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <metaverse/database.hpp>
#include <metaverse/node/define.hpp>

namespace libbitcoin {
namespace node {

/// A thread safe checkpoint queue.
class BCN_API check_list
{
public:
    /// The queue contains no checkpoints.
    bool empty() const;

    /// The number of checkpoints in the queue.
    size_t size() const;

    /// Reserve the entries indicated by the given heights.
    /// Any entry not reserved here will be ignored upon enqueue.
    void reserve(const database::block_database::heights& heights);

    /// Place a hash on the queue at the height if it has a reservation.
    void enqueue(hash_digest&& hash, size_t height);

    /// Remove the next entry by increasing height.
    bool dequeue(hash_digest& out_hash, size_t& out_height);

private:
    // A bidirection map is used for efficient hash and height retrieval.
    typedef boost::bimaps::bimap<
        boost::bimaps::unordered_set_of<hash_digest>,
        boost::bimaps::set_of<size_t>> checks;

    checks checks_;
    mutable shared_mutex mutex_;
};

} // namespace node
} // namespace libbitcoin

#endif
