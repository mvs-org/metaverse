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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MVS_CHAIN_HEADER_HPP
#define MVS_CHAIN_HEADER_HPP

#include <boost/multiprecision/cpp_int.hpp>
#include <functional>
#include <cstdint>
#include <istream>
#include <string>
#include <memory>
#include <vector>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/math/hash.hpp>
#include <metaverse/bitcoin/utility/data.hpp>
#include <metaverse/bitcoin/utility/reader.hpp>
#include <metaverse/bitcoin/utility/thread.hpp>
#include <metaverse/bitcoin/utility/writer.hpp>

#include <metaverse/consensus/libdevcore/FixedHash.h>
#include <metaverse/consensus/libdevcore/Common.h>
#include <metaverse/consensus/libdevcore/RLP.h>
#include <metaverse/consensus/libdevcore/SHA3.h>

namespace libbitcoin {
    using bigint = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<>>;
    using u64 =  boost::multiprecision::number<boost::multiprecision::cpp_int_backend<64, 64, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void>>;
    using u128 =  boost::multiprecision::number<boost::multiprecision::cpp_int_backend<128, 128, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void>>;
    using u256 =  boost::multiprecision::number<boost::multiprecision::cpp_int_backend<256, 256, boost::multiprecision::unsigned_magnitude, boost::multiprecision::unchecked, void>>;

namespace chain {

class BC_API header
{
public:
    typedef std::vector<header> list;
    typedef std::shared_ptr<header> ptr;
    typedef std::vector<ptr> ptr_list;

    static header factory_from_data(const data_chunk& data,
        bool with_transaction_count = true);
    static header factory_from_data(std::istream& stream,
        bool with_transaction_count = true);
    static header factory_from_data(reader& source,
        bool with_transaction_count = true);
    static uint64_t satoshi_fixed_size_without_transaction_count();

    header();
    header(const header& other);
    header(uint32_t version, const hash_digest& previous_block_hash,
        const hash_digest& merkle, uint32_t timestamp, const u256& bits,
        u64 nonce, const u256& mixhash, uint32_t number, uint64_t transaction_count=0);

    header(header&& other);
    header(uint32_t version, hash_digest&& previous_block_hash,
        hash_digest&& merkle, uint32_t timestamp, const u256& bits,
        u64 nonce, const u256& mixhash, uint32_t number, uint64_t transaction_count=0);

    /// This class is move assignable but not copy assignable.
    header& operator=(header&& other);

    // TODO: eliminate blockchain transaction copies and then delete this.
    header& operator=(const header& other) /*= delete*/;

    bool from_data(const data_chunk& data, bool with_transaction_count = true);
    bool from_data(std::istream& stream, bool with_transaction_count = true);
    bool from_data(reader& source, bool with_transaction_count = true);
    data_chunk to_data(bool with_transaction_count = true) const;
    void to_data(std::ostream& stream, bool with_transaction_count = true) const;
    void to_data(writer& sink, bool with_transaction_count = true) const;
    hash_digest hash() const;
    bool is_valid() const;
    void reset();
    uint64_t serialized_size(bool with_transaction_count = true) const;

    uint32_t version;
    hash_digest previous_block_hash;
    hash_digest merkle;
    uint32_t timestamp;
    u256 bits;
    u64 nonce;
    u256 mixhash;
    uint32_t number;

    // The longest size (64) of a protocol variable int is deserialized here.
    // WHen writing a block the size of the transaction collection is used.
    uint64_t transaction_count;

private:
    mutable upgrade_mutex mutex_;
    mutable std::shared_ptr<hash_digest> hash_;
};

BC_API bool operator==(const header& left, const header& right);
BC_API bool operator!=(const header& left, const header& right);

} // namespace chain
} // namespace libbitcoin

#endif
