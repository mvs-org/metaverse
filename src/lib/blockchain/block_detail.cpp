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
#include <metaverse/blockchain/block_detail.hpp>

#include <cstdint>
#include <memory>
#include <utility>
#include <metaverse/bitcoin.hpp>

namespace libbitcoin {
namespace blockchain {

using namespace message;

// Use zero as orphan sentinel since this is precluded by the orphan pool.
static constexpr auto orphan_height = 0u;

block_detail::block_detail(block_ptr actual_block)
  : code_(error::success),
    processed_(false),
    height_(orphan_height),
    actual_block_(actual_block),
    is_checked_work_proof_(false)
{
}

// Hand off ownership of a block to this wrapper.
block_detail::block_detail(chain::block&& actual_block)
  : block_detail(std::make_shared<block_message>(actual_block))
{
}

block_detail::block_ptr block_detail::actual() const
{
    return actual_block_;
}

void block_detail::set_processed()
{
    processed_.store(true);
}

bool block_detail::processed() const
{
    return processed_.load();
}

void block_detail::set_height(uint64_t height)
{
    BITCOIN_ASSERT(height != orphan_height);
    height_.store(height);
}

uint64_t block_detail::height() const
{
    return height_.load();
}

void block_detail::set_error(const code& code)
{
    code_.store(code);
}

code block_detail::error() const
{
    return code_.load();
}

const hash_digest block_detail::hash() const
{
    // This relies on the hash caching optimization.
    return actual_block_->header.hash();
}

} // namespace blockchain
} // namespace libbitcoin
