/**
 * Copyright (c) 2011-2016 libbitcoin developers (see AUTHORS)
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
#ifndef MVS_NODE_HEADER_LIST_HPP
#define MVS_NODE_HEADER_LIST_HPP

#include <cstddef>
#include <memory>
#include <metaverse/bitcoin.hpp>
#include <metaverse/node/define.hpp>
#include <metaverse/node/utility/check_list.hpp>

namespace libbitcoin {
namespace node {

typedef bc::message::headers::const_ptr headers_const_ptr;

/// A smart queue for chaining blockchain headers, thread safe.
/// The peer should be stopped if merge fails.
class BCN_API header_list
{
public:
    typedef std::shared_ptr<header_list> ptr;

    /// Construct a list to fill the specified range of headers.
    header_list(size_t slot, const config::checkpoint& start,
        const config::checkpoint& stop);

    /// The list is fully populated.
    bool complete() const;

    /// The slot id of this instance.
    size_t slot() const;

    /// The height of the first header in the list.
    size_t first_height() const;

    /// The height of the last header in the list (or the start height).
    size_t previous_height() const;

    /// The hash of the last header in the list (or the start hash).
    hash_digest previous_hash() const;

    /// The hash of the stop checkpoint.
    const hash_digest& stop_hash() const;

    /// The ordered list of headers.
    /// This is not thread safe, call only after complete.
    const chain::header::list& headers() const;

    /////// Generate a check list from a complete list of headers.
    ////config::checkpoint::list to_checkpoints() const;

    /// Merge the hashes in the message with those in the queue.
    /// Return true if linked all headers or complete.
    bool merge(bc::message::headers::const_ptr message);

private:
    // The number of headers remaining until complete.
    size_t remaining() const;

    /// The hash of the last header in the list (or the start hash).
    const hash_digest& last() const;

    // Determine if the hash is linked to the preceding header.
    bool link(const chain::header& header) const;

    // Determine if the header is valid (context free).
    bool check(const chain::header& header) const;

    // Determine if the header is acceptable for the current height.
    bool accept(const chain::header& header) const;

    // This is protected by mutex.
    chain::header::list list_;
    mutable upgrade_mutex mutex_;

    const size_t height_;
    const config::checkpoint start_;
    const config::checkpoint stop_;
    const size_t slot_;
};

} // namespace node
} // namespace libbitcoin

#endif

