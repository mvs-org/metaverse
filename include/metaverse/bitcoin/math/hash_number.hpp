/**
 * Copyright (c) 2011-2021 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2021 metaverse core developers (see MVS-AUTHORS)
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
#ifndef MVS_HASH_NUMBER_HPP
#define MVS_HASH_NUMBER_HPP

#include <cstddef>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/math/hash.hpp>
#include <metaverse/bitcoin/math/uint256.hpp>

namespace libbitcoin {

/**
 * Represents a target hash or proof of work sum.
 * Used for block proof of works to calculate whether they reach
 * a certain target or which chain is longest.
 */
class BC_API hash_number
{
public:
    hash_number();
    hash_number(uint64_t value);
    // Returns false if negative or overflowed.
    bool set_compact(uint32_t compact);
    uint32_t compact() const;
    void set_hash(const hash_digest& hash);
    hash_digest hash() const;

    const hash_number operator~() const;

    // int64_t resolves to this in Satoshi's GetNextWorkRequired()
    hash_number& operator*=(uint32_t value);
    hash_number& operator/=(uint32_t value);
    hash_number& operator<<=(uint32_t shift);

    hash_number& operator/=(const hash_number& number_b);
    hash_number& operator+=(const hash_number& number_b);

private:
    friend bool operator>(
        const hash_number& number_a, const hash_number& number_b);
    friend bool operator<=(
        const hash_number& number_a, const hash_number& number_b);
    friend const hash_number operator<<(
        const hash_number& number_a, int shift);
    friend const hash_number operator/(
        const hash_number& number_a, const hash_number& number_b);
    friend const hash_number operator+(
        const hash_number& number_a, const hash_number& number_b);
    friend bool operator==(
        const hash_number& number, uint64_t value);

    uint256_t hash_;
};

} // namespace libbitcoin

#endif
