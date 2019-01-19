/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
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
#include <metaverse/database/result/block_result.hpp>

#include <cstdint>
#include <cstddef>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/memory/memory.hpp>

namespace libbitcoin {
namespace database {

using namespace bc::chain;

static constexpr size_t header_size = 148;
static constexpr size_t height_size = sizeof(uint32_t);
static constexpr size_t count_size = sizeof(uint32_t);

block_result::block_result(const memory_ptr slab)
  : slab_(slab)
{
}

block_result::operator bool() const
{
    return slab_ != nullptr;
}

chain::header block_result::header() const
{
    BITCOIN_ASSERT(slab_);
    chain::header header;
    const auto memory = REMAP_ADDRESS(slab_);
    auto deserial = make_deserializer_unsafe(memory);
    header.from_data(deserial, false);
    return header;
    //// return deserialize_header(memory, size_limit_);
}

size_t block_result::height() const
{
    BITCOIN_ASSERT(slab_);
    const auto memory = REMAP_ADDRESS(slab_);
    return from_little_endian_unsafe<uint32_t>(memory + header_size);
}

size_t block_result::transaction_count() const
{
    BITCOIN_ASSERT(slab_);
    const auto memory = REMAP_ADDRESS(slab_);
    const auto offset = header_size + height_size;
    return from_little_endian_unsafe<uint32_t>(memory + offset);
}

hash_digest block_result::transaction_hash(size_t index) const
{
    BITCOIN_ASSERT(slab_);
    BITCOIN_ASSERT(index < transaction_count());
    const auto memory = REMAP_ADDRESS(slab_);
    const auto offset = header_size + height_size + count_size;
    const auto first = memory + offset + index * hash_size;
    auto deserial = make_deserializer_unsafe(first);
    return deserial.read_hash();
}

ec_signature block_result::blocksig() const
{
    if (!(header().is_proof_of_stake() || header().is_proof_of_dpos()))
        return ec_signature();

    BITCOIN_ASSERT(slab_);
    const auto memory = REMAP_ADDRESS(slab_);

    const auto offset = header_size + height_size + count_size;
    const auto first = memory + offset + transaction_count() * hash_size;
    auto deserial = make_deserializer_unsafe(first);
    return deserial.read_bytes<ec_signature_size>();
}

ec_compressed block_result::public_key() const
{
    if (!header().is_proof_of_dpos())
        return ec_compressed();

    BITCOIN_ASSERT(slab_);
    const auto memory = REMAP_ADDRESS(slab_);

    const auto offset = header_size + height_size + count_size;
    const auto first = memory + offset + transaction_count() * hash_size + ec_signature_size;
    auto deserial = make_deserializer_unsafe(first);
    return deserial.read_bytes<ec_compressed_size>();
}

} // namespace database
} // namespace libbitcoin
