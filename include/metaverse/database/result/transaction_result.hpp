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
#ifndef MVS_DATABASE_TRANSACTION_RESULT_HPP
#define MVS_DATABASE_TRANSACTION_RESULT_HPP

#include <cstddef>
#include <cstdint>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/define.hpp>
#include <metaverse/database/memory/memory.hpp>

namespace libbitcoin {
namespace database {
    
/// Deferred read transaction result.
class BCD_API transaction_result
{
public:
    transaction_result(const memory_ptr slab);

    /// True if this transaction result is valid (found).
    operator bool() const;

    /// The height of the block which includes the transaction.
    size_t height() const;

    /// The position of the transaction within its block.
    size_t index() const;

    /// The transaction.
    chain::transaction transaction() const;

private:
    const memory_ptr slab_;
};

} // namespace database
} // namespace libbitcoin

#endif
