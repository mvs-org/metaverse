/**
 * Copyright (c) 2011-2016 libbitcoin developers (see AUTHORS)
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
#ifndef LIBBITCOIN_NODE_PERFORMANCE_HPP
#define LIBBITCOIN_NODE_PERFORMANCE_HPP

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <bitcoin/blockchain.hpp>
#include <bitcoin/node/define.hpp>

namespace libbitcoin {
namespace node {

class BCN_API performance
{
public:

    /// The normalized rate derived from the performance values.
    double normal() const;

    /// The rate derived from the performance values (inclusive of store cost).
    double total() const;

    /// The ratio of database time to total time.
    double ratio() const;

    bool idle;
    size_t events;
    uint64_t database;
    uint64_t window;
};

// Coerce division into double and error into zero.
template<typename Quotient, typename Dividend, typename Divisor>
static Quotient divide(Dividend dividend, Divisor divisor)
{
    const auto quotient = static_cast<Quotient>(dividend) / divisor;
    return std::isnan(quotient) || std::isinf(quotient) ? 0.0 : quotient;
}

} // namespace node
} // namespace libbitcoin

#endif

