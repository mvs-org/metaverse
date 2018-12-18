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
#ifndef MVS_CHAIN_POINT_HPP
#define MVS_CHAIN_POINT_HPP

#include <cstdint>
#include <istream>
#include <string>
#include <vector>
#include <boost/functional/hash.hpp>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/chain/point_iterator.hpp>
#include <metaverse/bitcoin/math/hash.hpp>
#include <metaverse/bitcoin/utility/data.hpp>
#include <metaverse/bitcoin/utility/reader.hpp>
#include <metaverse/bitcoin/utility/writer.hpp>
#include <metaverse/bitcoin/base_primary.hpp>

namespace libbitcoin {
namespace chain {

class BC_API point
    : public base_primary<point>
{
public:
    typedef std::vector<point> list;
    typedef std::vector<uint32_t> indexes;

    static uint64_t satoshi_fixed_size();

    point();

    point(point&& other);
    point(const point& other);

    point(hash_digest&& hash, uint32_t index);
    point(const hash_digest& hash, uint32_t index);

    virtual ~point();


    // Operators.
    //-------------------------------------------------------------------------

    /// This class is move assignable and copy assignable.
    point& operator=(point&& other);
    point& operator=(const point& other);

    bool operator<(const point& other) const;
    bool operator==(const point& other) const;
    bool operator!=(const point& other) const;

    uint64_t checksum() const;

    bool is_null() const;

    bool from_data_t(reader& source);
    void to_data_t(writer& sink) const;

    std::string to_string() const;
    bool is_valid() const;
    void reset();
    uint64_t serialized_size() const;

    point_iterator begin() const;
    point_iterator end() const;

    hash_digest hash;
    uint32_t index;
};

// BC_API bool operator==(const point& left, const point& right);
// BC_API bool operator!=(const point& left, const point& right);

// BC_API bool operator<(const point& left, const point& right);

typedef point input_point;
// typedef point output_point;


} // namespace chain
} // namespace libbitcoin

using input_point = libbitcoin::chain::input_point;

namespace std
{

// Extend std namespace with our hash wrapper, used as database hash.
template <>
struct hash<bc::chain::point>
{
    // Changes to this function invalidate existing database files.
    size_t operator()(const bc::chain::point& point) const
    {
        size_t seed = 0;
        boost::hash_combine(seed, point.hash);
        boost::hash_combine(seed, point.index);
        return seed;
    }
};

// Extend std namespace with the size of our point, used as database key size.
template <>
struct tuple_size<bc::chain::point>
{
    static const size_t value = std::tuple_size<bc::hash_digest>::value +
        sizeof(uint32_t);

    operator std::size_t() const
    {
        return value;
    }
};

} // namespace std

#endif
