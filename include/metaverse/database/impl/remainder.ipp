/**
 * Copyright (c) 2019-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2019-2020 metaverse core developers (see MVS-AUTHORS)
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
#ifndef MVS_DATABASE_REMAINDER_IPP
#define MVS_DATABASE_REMAINDER_IPP

#include <cstdint>
#include <functional>
#include <metaverse/bitcoin.hpp>

namespace libbitcoin {
namespace database {

/// Return a hash of the key reduced to the domain of the divisor.
template <typename KeyType, typename Divisor>
Divisor remainder(const KeyType& key, const Divisor divisor)
{
    return divisor == 0 ? 0 : std::hash<KeyType>()(key) % divisor;
}

} // namespace database
} // namespace libbitcoin

#endif
