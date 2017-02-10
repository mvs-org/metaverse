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
#include <bitcoin/database/result/transaction_result.hpp>

#include <cstddef>
#include <cstdint>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/database/memory/memory.hpp>

namespace libbitcoin {
namespace database {

using namespace bc::chain;

static constexpr size_t height_size = sizeof(uint32_t);
static constexpr size_t index_size = sizeof(uint32_t);

template <typename Iterator>
chain::transaction deserialize_tx(const Iterator first)
{
    chain::transaction tx;
    auto deserial = make_deserializer_unsafe(first);
    tx.from_data(deserial);
    return tx;
}

transaction_result::transaction_result(const memory_ptr slab)
  : slab_(slab)
{
}

transaction_result::operator bool() const
{
    return slab_ != nullptr;
}

size_t transaction_result::height() const
{
    BITCOIN_ASSERT(slab_);
    const auto memory = REMAP_ADDRESS(slab_);
    return from_little_endian_unsafe<uint32_t>(memory);
}

size_t transaction_result::index() const
{
    BITCOIN_ASSERT(slab_);
    const auto memory = REMAP_ADDRESS(slab_);
    return from_little_endian_unsafe<uint32_t>(memory + height_size);
}

chain::transaction transaction_result::transaction() const
{
    BITCOIN_ASSERT(slab_);
    const auto memory = REMAP_ADDRESS(slab_);
    return deserialize_tx(memory + height_size + index_size);
    //// return deserialize_tx(memory + 8, size_limit_ - 8);
}
} // namespace database
} // namespace libbitcoin
