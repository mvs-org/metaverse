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
#ifndef MVS_DATABASE_BLOCK_RESULT_HPP
#define MVS_DATABASE_BLOCK_RESULT_HPP

#include <cstdint>
#include <cstddef>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/database/define.hpp>
#include <bitcoin/database/memory/memory.hpp>

namespace libbitcoin {
namespace database {

/// Deferred read block result.
class BCD_API block_result
{
public:
    block_result(const memory_ptr slab);

    /// True if this block result is valid (found).
    operator bool() const;

    /// The block header.
    chain::header header() const;

    /// The height of this block in the chain.
    size_t height() const;

    /// The number of transactions in this block.
    size_t transaction_count() const;

    /// A transaction hash where index < transaction_count.
    hash_digest transaction_hash(size_t index) const;

private:
    const memory_ptr slab_;
};

} // namespace database
} // namespace libbitcoin

#endif
