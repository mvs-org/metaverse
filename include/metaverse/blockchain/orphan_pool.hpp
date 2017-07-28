/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS)
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
#ifndef MVS_BLOCKCHAIN_orphan_pool_HPP
#define MVS_BLOCKCHAIN_orphan_pool_HPP

#include <cstddef>
#include <memory>
#include <boost/circular_buffer.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/blockchain/define.hpp>
#include <metaverse/blockchain/block_detail.hpp>

namespace libbitcoin {
namespace blockchain {

/// This class is thread safe.
/// An unordered memory pool for orphan blocks.
class BCB_API orphan_pool
{
public:
    typedef std::shared_ptr<orphan_pool> ptr;

    orphan_pool(size_t capacity);

    /// Add a block to the pool.
    bool add(block_detail::ptr block);

    /// Remove a block from the pool.
    void remove(block_detail::ptr block);

    /// Obtain blocks from the pool
    void fetch_all(block_detail::list& blocks) const;

    /// Remove from the message all vectors that match orphans.
    void filter(message::get_data::ptr message) const;

    /// Get the longest connected chain of orphans after 'end'.
    block_detail::list trace(block_detail::ptr end) const;

    /// Get the set of unprocessed orphans.
    block_detail::list unprocessed() const;

    bool add_pending_block(const hash_digest& needed_block, const block_detail::ptr& pending_block);
    block_detail::ptr delete_pending_block(const hash_digest& needed_block);

private:
//    typedef boost::circular_buffer<block_detail::ptr> buffer;
    typedef std::vector<block_detail::ptr> buffer;
    typedef buffer::const_iterator const_iterator;
    typedef buffer::const_reverse_iterator const_reverse_iterator;

    bool exists(const hash_digest& hash) const;
    bool exists(const chain::header& header) const;
    const_iterator find(buffer::const_iterator begin, const hash_digest& hash) const;
    const_reverse_iterator rfind(buffer::const_reverse_iterator begin, const hash_digest& hash) const;

    // The buffer is protected by mutex.
    buffer buffer_;
    mutable upgrade_mutex mutex_;

    std::multimap<hash_digest, block_detail::ptr> pending_blocks_;
    std::set<hash_digest> pending_blocks_hash_;
};

} // namespace blockchain
} // namespace libbitcoin

#endif
