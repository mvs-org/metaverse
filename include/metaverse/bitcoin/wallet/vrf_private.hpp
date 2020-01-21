/**
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MVS_WALLET_vrf_private_KEY_HPP
#define MVS_WALLET_vrf_private_KEY_HPP

#include <cstdint>
#include <string>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/utility/data.hpp>
#include <sodium/crypto_vrf.h>
#include <metaverse/bitcoin/compat.hpp>

namespace libbitcoin {
namespace wallet {

// 
BC_CONSTEXPR size_t vrf_public_size = crypto_vrf_PUBLICKEYBYTES;
typedef byte_array<vrf_public_size> vrf_public;

BC_CONSTEXPR size_t vrf_secret_size = crypto_vrf_SECRETKEYBYTES;
typedef byte_array<vrf_secret_size> vrf_secret;

BC_CONSTEXPR size_t vrf_proof_size = crypto_vrf_PROOFBYTES;
typedef byte_array<vrf_proof_size> vrf_proof;

BC_CONSTEXPR size_t vrf_hash_size = crypto_vrf_OUTPUTBYTES;
typedef byte_array<vrf_hash_size> vrf_hash;

BC_CONSTEXPR size_t vrf_seed_size = crypto_vrf_SEEDBYTES;
typedef byte_array<vrf_seed_size> vrf_seed;

/// An extended private key, as defined by BIP 32.
class BC_API vrf_private
{
public:
    /// Constructors.
    vrf_private();
    vrf_private(const vrf_private& other);
    vrf_private(const data_chunk& data);
    vrf_private(const vrf_seed & seed);


    /// Operators.
    bool operator<(const vrf_private& other) const;
    bool operator==(const vrf_private& other) const;
    bool operator!=(const vrf_private& other) const;
    vrf_private& operator=(const vrf_private& other);
    friend std::istream& operator>>(std::istream& in, vrf_private& to);
    friend std::ostream& operator<<(std::ostream& out,
        const vrf_private& of);

    /// Cast operators.
    operator const vrf_secret&() const;

    /// Serializer.
    std::string encoded() const;

    /// Accessors.
    const vrf_secret& secret() const;

    /// Methods.
    vrf_public to_public() const;

    bool prove(vrf_proof& proof, const data_chunk & m);

    ///verify
    //prove to hash
    static bool proof_to_hash(vrf_hash& result, const vrf_proof& proof);
    //verify proof and pk ,return "result'"
    static bool verify(vrf_hash& result, const vrf_proof& proof,
                       const vrf_public& pk, const data_chunk & m);
private:
    vrf_secret secret_;
};

} // namespace wallet
} // namespace libbitcoin

#endif
