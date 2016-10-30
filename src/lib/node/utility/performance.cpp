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
#include <bitcoin/node/utility/performance.hpp>

#include <bitcoin/bitcoin.hpp>

namespace libbitcoin {
namespace node {

double performance::normal() const
{
    // If numerator is small we can overflow (infinity).
    return divide<double>(events, static_cast<double>(window) - database);
}

double performance::total() const
{
    return divide<double>(events, window);
}

double performance::ratio() const
{
    return divide<double>(database, window);
}

} // namespace node
} // namespace libbitcoin
