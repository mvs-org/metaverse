/**
 * Copyright (c) 2011-2015 metaverse developers (see AUTHORS)
 *
 * This file is part of mvs-node.
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
#include <bitcoin/database/result/asset_result.hpp>
#include <bitcoin/bitcoin/chain/attachment/asset/asset_detail.hpp>
#include <cstddef>
#include <cstdint>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/database/memory/memory.hpp>

namespace libbitcoin {
namespace database {

template <typename Iterator>
asset_detail deserialize_asset_detail(const Iterator first)
{
    asset_detail sp_detail;
    auto deserial = make_deserializer_unsafe(first);
    sp_detail.from_data(deserial);
    return sp_detail;
}

asset_result::asset_result(const memory_ptr slab)
  : base_result(slab)
{
}

asset_detail asset_result::get_asset_detail() const
{
    BITCOIN_ASSERT(get_slab());
    const auto memory = REMAP_ADDRESS(get_slab());
    return deserialize_asset_detail(memory);
}
} // namespace database
} // namespace libbitcoin
