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
#include <metaverse/lib/database/result/account_asset_result.hpp>
#include <metaverse/lib/bitcoin/chain/attachment/asset/asset_transfer.hpp>
#include <cstddef>
#include <cstdint>
#include <metaverse/lib/bitcoin.hpp>
#include <metaverse/lib/database/memory/memory.hpp>

namespace libbitcoin {
namespace database {

template <typename Iterator>
asset_transfer deserialize_asset_transfer(const Iterator first)
{
    asset_transfer sp_detail;
    auto deserial = make_deserializer_unsafe(first);
    sp_detail.from_data(deserial);
    return sp_detail;
}

account_asset_result::account_asset_result(const memory_ptr slab)
  : base_result(slab)
{
}

asset_transfer account_asset_result::get_account_asset_transfer() const
{
    BITCOIN_ASSERT(get_slab());
    const auto memory = REMAP_ADDRESS(get_slab());
    return deserialize_asset_transfer(memory);
}
} // namespace database
} // namespace libbitcoin
