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
#ifndef MVS_CHAIN_OUTPUT_POINT_HPP
#define MVS_CHAIN_OUTPUT_POINT_HPP

#include <cstddef>
#include <cstdint>
#include <vector>
#include <metaverse/bitcoin/chain/output.hpp>
#include <metaverse/bitcoin/chain/point.hpp>
#include <metaverse/bitcoin/define.hpp>

namespace libbitcoin {
namespace chain {

class BC_API output_point
  : public point
{
public:

    // THIS IS FOR LIBRARY USE ONLY, DO NOT CREATE A DEPENDENCY ON IT.
    struct validation
    {
        /// Must be false if confirmed is false.
        /// output spender's tx->block is indexed or confirmed not above fork.
        bool spent = false;

        /// The output->tx is confirmed|indexed, fork point dependent.
        bool confirmed = false;

        /// The previous output is a coinbase (must verify spender maturity).
        bool coinbase = false;

        /// Prevout height is used for coinbase maturity and relative lock time.
        size_t height = 0;

        /// Median time past is used for relative lock time.
        uint32_t median_time_past = 0;

        /// The output cache contains the output referenced by the input point.
        /// If the cache.value is not_found (default) the output is not found.
        output cache;
    };

    // Constructors.
    //-------------------------------------------------------------------------

    output_point();

    output_point(point&& other);
    output_point(const point& value);

    output_point(output_point&& other);
    output_point(const output_point& other);

    output_point(hash_digest&& hash, uint32_t index);
    output_point(const hash_digest& hash, uint32_t index);

    // Operators.
    //-------------------------------------------------------------------------
    // This class is move assignable and copy assignable.

    output_point& operator=(point&& other);
    output_point& operator=(const point&);
    output_point& operator=(output_point&& other);
    output_point& operator=(const output_point&);

    bool operator==(const point& other) const;
    bool operator!=(const point& other) const;
    bool operator==(const output_point& other) const;
    bool operator!=(const output_point& other) const;

    // Deserialization.
    //-------------------------------------------------------------------------

    static output_point factory(const data_chunk& data);
    static output_point factory(std::istream& stream);
    static output_point factory(reader& source);

    // Validation.
    //-------------------------------------------------------------------------

    /// True if cached previous output is mature enough to spend from height.
    bool is_mature(size_t height) const;

    // THIS IS FOR LIBRARY USE ONLY, DO NOT CREATE A DEPENDENCY ON IT.
    mutable validation metadata;

protected:
    // So that input may call reset from its own.
    friend class input;
};

struct BC_API output_info
{
    typedef std::vector<output_info> list;

    output_point point;
    uint64_t value;
};

struct BC_API points_info
{
    output_point::list points;
    uint64_t change;
};


} // namespace chain
} // namespace libbitcoin

using output_point = libbitcoin::chain::output_point;

#endif
