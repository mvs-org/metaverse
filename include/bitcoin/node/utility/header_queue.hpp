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
#ifndef LIBBITCOIN_NODE_HEADER_QUEUE_HPP
#define LIBBITCOIN_NODE_HEADER_QUEUE_HPP

#include <cstddef>
#include <vector>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/node/define.hpp>

namespace libbitcoin {
namespace node {

// A smart queue for chaining blockchain headers, thread safe.
class BCN_API header_queue
{
public:

    /// True if the specified hash is marked as removed.
    static bool valid(const hash_digest& hash);

    /// Construct a block hash list with specified height offset.
    header_queue(const config::checkpoint::list& checkpoints);

    /// True if the list is empty.
    bool empty() const;

    /// The number of elements.
    size_t size() const;

    /// The height of the first element.
    size_t first_height() const;

    /// The height of the last element.
    size_t last_height() const;

    /// The last hash in the list or null_hash if none.
    hash_digest last_hash() const;

    /// Remove the first count of hashes, return true if satisfied.
    bool dequeue(size_t count=1);

    /// Obtain and remove the first hash.
    bool dequeue(hash_digest& out_hash, size_t& out_height);

    /// Merge the hashes in the message with those in the queue.
    bool enqueue(message::headers::ptr message);

    /// Clear the queue and populate the hash at the given height.
    void initialize(const config::checkpoint& check);

    /// Clear the queue and populate the hash at the given height.
    void initialize(const hash_digest& hash, size_t height);

    /// Mark the heights if they exist.
    void invalidate(size_t first_height, size_t count);

private:
    // True if the queue is empty (not locked).
    bool is_empty() const;

    // The number of hashes in the queue (not locked).
    size_t get_size() const;

    // The last hash in the list or null_hash if none (not locked).
    size_t last() const;

    // Roll back the list to the last checkpoint.
    void rollback();

    // Merge a list of block hashes to the list, validating linkage.
    bool merge(const chain::header::list& headers);

    // Determine if the hash violates a checkpoint.
    bool check(const hash_digest& hash, size_t height) const;

    // Determine if the hash is linked to the give (preceding) header.
    bool linked(const chain::header& header, const hash_digest& hash) const;

    // The list of checkpoints that determines the sync range.
    const config::checkpoint::list& checkpoints_;

    // protected by mutex.
    size_t height_;
    hash_list list_;
    hash_list::iterator head_;
    mutable upgrade_mutex mutex_;
};

} // namespace node
} // namespace libbitcoin

#endif

