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
#ifndef MVS_DATABASE_BASE_RESULT_HPP
#define MVS_DATABASE_BASE_RESULT_HPP

#include <cstddef>
#include <cstdint>
#include <metaverse/lib/bitcoin.hpp>
#include <metaverse/lib/database/define.hpp>
#include <metaverse/lib/database/memory/memory.hpp>

namespace libbitcoin {
namespace database {
    
/// Deferred read base result.
class BCD_API base_result
{
public:
    base_result(const memory_ptr slab): slab_(slab)
	{
	}

    /// True if this base result is valid (found).
    operator bool() const
	{
		return slab_ != nullptr;
	}
	memory_ptr get_slab() const
	{
		return slab_;
	}
private:
    const memory_ptr slab_;
};
#if 0
base_result::base_result(const memory_ptr slab)
  : slab_(slab)
{
}

base_result::operator bool() const
{
    return slab_ != nullptr;
}

memory_ptr base_result::get_slab() const
{
	return slab_;
}
#endif

} // namespace database
} // namespace libbitcoin

#endif
