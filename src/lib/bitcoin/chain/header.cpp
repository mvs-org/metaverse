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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/bitcoin/chain/header.hpp>

#include <utility>
#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/constants.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>
#include <metaverse/consensus/libdevcore/FixedHash.h>

namespace libbitcoin {
namespace chain {

uint64_t header::satoshi_fixed_size_without_transaction_count()
{
    return 148;
}

header::header()
    : header(0, null_hash, null_hash, 0, 0, 0, 0, 0, 0)
{
}

header::header(const header& other)
  : header(other.version, other.previous_block_hash, other.merkle,
        other.timestamp, other.bits, other.nonce, other.mixhash, other.number, other.transaction_count)
{
}

header::header(uint32_t version, const hash_digest& previous_block_hash,
    const hash_digest& merkle, uint32_t timestamp, const u256& bits,
    u64 nonce, const u256& mixhash, uint32_t number, uint64_t transaction_count)
  : version(version),
    previous_block_hash(previous_block_hash),
    merkle(merkle),
    timestamp(timestamp),
    bits(bits),
    nonce(nonce),
    mixhash(mixhash),
    number(number),
    transaction_count(transaction_count),
    hash_(nullptr)
{
}

header::header(header&& other)
  : header(other.version, std::forward<hash_digest>(other.previous_block_hash),
        std::forward<hash_digest>(other.merkle), other.timestamp, other.bits,
        other.nonce, other.mixhash, other.number, other.transaction_count)
{
}

header::header(uint32_t version, hash_digest&& previous_block_hash,
    hash_digest&& merkle, uint32_t timestamp, const u256& bits, u64 nonce, const u256& mixhash, uint32_t number,
    uint64_t transaction_count)
  : version(version),
    previous_block_hash(std::forward<hash_digest>(previous_block_hash)),
    merkle(std::forward<hash_digest>(merkle)),
    timestamp(timestamp),
    bits(bits),
    nonce(nonce),
    mixhash(mixhash),
    number(number),
    transaction_count(transaction_count),
    hash_(nullptr)
{
}

header& header::operator=(header&& other)
{
    version = other.version;
    previous_block_hash = std::move(other.previous_block_hash);
    merkle = std::move(other.merkle);
    timestamp = other.timestamp;
    bits = other.bits;
    nonce = other.nonce;
    mixhash = other.mixhash;
    number = other.number;
    transaction_count = other.transaction_count;
    return *this;
}

// TODO: eliminate header copies and then delete this.
header& header::operator=(const header& other)
{
    version = other.version;
    previous_block_hash = other.previous_block_hash;
    merkle = other.merkle;
    timestamp = other.timestamp;
    bits = other.bits;
    nonce = other.nonce;
    mixhash = other.mixhash;
    number = other.number;
    transaction_count = other.transaction_count;
    return *this;
}

bool header::is_valid() const
{
    return (version != 0) ||
        (previous_block_hash != null_hash) ||
        (merkle != null_hash) ||
        (timestamp != 0) ||
        (bits != 0) ||
        (nonce != 0);
}

bool header::is_proof_of_stake() const
{
    return version == block_version_pos;
}

bool header::is_proof_of_work() const
{
    return version == block_version_pow;
}

bool header::is_proof_of_dpos() const
{
    return version == block_version_dpos;
}

void header::reset()
{
    version = 0;
    previous_block_hash.fill(0);
    merkle.fill(0);
    timestamp = 0;
    bits = 0;
    nonce = 0;

    mutex_.lock();
    hash_.reset();
    mutex_.unlock();
}

bool header::from_data_t(reader& source, bool with_transaction_count)
{
    reset();

    version = source.read_4_bytes_little_endian();
    previous_block_hash = source.read_hash();
    merkle = source.read_hash();
    timestamp = source.read_4_bytes_little_endian();

    unsigned char buff[32];
    source.read_data(buff, 32);
    bits = (h256::Arith)(h256((const uint8_t*)&buff[0], h256::ConstructFromPointer));

    source.read_data(buff, 8);
    nonce = (h64::Arith)(h64((const uint8_t*)&buff[0], h64::ConstructFromPointer));

    source.read_data(buff, 32);
    mixhash = (h256::Arith)(h256((const uint8_t*)&buff[0], h256::ConstructFromPointer));

    number = source.read_4_bytes_little_endian();

    transaction_count = 0;
    if (with_transaction_count)
        transaction_count = source.read_variable_uint_little_endian();

    const auto result = static_cast<bool>(source);

    if (!result)
        reset();

    return result;
}


void header::to_data_t(writer& sink, bool with_transaction_count) const
{
    sink.write_4_bytes_little_endian(version);
    sink.write_hash(previous_block_hash);
    sink.write_hash(merkle);
    sink.write_4_bytes_little_endian(timestamp);

    sink.write_data((unsigned char*)h256(bits).data(), 32);
    sink.write_data((unsigned char*)h64(nonce).data(), 8);
    sink.write_data((unsigned char*)h256(mixhash).data(), 32);

    sink.write_4_bytes_little_endian(number);

    if (with_transaction_count)
        sink.write_variable_uint_little_endian(transaction_count);
}

uint64_t header::serialized_size(bool with_transaction_count) const
{
    auto size = satoshi_fixed_size_without_transaction_count();

    if (with_transaction_count)
        size += variable_uint_size(transaction_count);

    return size;
}

hash_digest header::hash() const
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_upgrade();

    if (!hash_)
    {
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        mutex_.unlock_upgrade_and_lock();
        hash_.reset(new hash_digest(bitcoin_hash(to_data(false))));
        mutex_.unlock_and_lock_upgrade();
        //---------------------------------------------------------------------
    }

    hash_digest hash = *hash_;
    mutex_.unlock_upgrade();
    ///////////////////////////////////////////////////////////////////////////

    return hash;
}

bool operator==(const header& left, const header& right)
{
    return (left.version == right.version)
        && (left.previous_block_hash == right.previous_block_hash)
        && (left.merkle == right.merkle)
        && (left.timestamp == right.timestamp)
        && (left.bits == right.bits)
        && (left.nonce == right.nonce)
        && (left.transaction_count == right.transaction_count);
}

bool operator!=(const header& left, const header& right)
{
    return !(left == right);
}

std::string get_block_version(const header& header)
{
    return get_block_version(header.version);
}

std::string get_block_version(block_version version)
{
    return get_block_version((uint32_t)version);
}

std::string get_block_version(uint32_t version)
{
    switch (version) {
    case block_version_any:
        return " Any";
    case block_version_pow:
        return " PoW";
    case block_version_pos:
        return " PoS";
    case block_version_dpos:
        return "DPoS";
    default:;
    }
    return "Unknown";
}

} // namspace chain
} // namspace libbitcoin
