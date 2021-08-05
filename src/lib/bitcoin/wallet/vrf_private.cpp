/**
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
#include <metaverse/bitcoin/wallet/vrf_private.hpp>

#include <cstdint>
#include <string>
#include <metaverse/bitcoin/math/hash.hpp>
#include <metaverse/bitcoin/wallet/vrf_private.hpp>
#include <metaverse/bitcoin/formats/base_16.hpp>

namespace libbitcoin {
namespace wallet {


vrf_private::vrf_private()
{
    vrf_public pk;
    sodium::crypto_vrf_keypair(pk.data(), secret_.data());
}

vrf_private::vrf_private(const vrf_private& other)
: secret_(other.secret_)
{
}

vrf_private::vrf_private(const data_chunk& data)
{
    auto seed = sha256_hash(data);
    vrf_public pk;
    sodium::crypto_vrf_keypair_from_seed(pk.data(), secret_.data(), seed.data());
}

// This reads the private version and sets the public to mainnet.
vrf_private::vrf_private(const vrf_seed& seed)
{
    vrf_public pk;
    sodium::crypto_vrf_keypair_from_seed(pk.data(), secret_.data(), seed.data());
}

// Cast operators.
// ----------------------------------------------------------------------------

vrf_private::operator const vrf_secret&() const
{
    return secret_;
}

// Serializer.
// ----------------------------------------------------------------------------

/// Accessors.
// ----------------------------------------------------------------------------

const vrf_secret& vrf_private::secret() const
{
    return secret_;
}

// Methods.
// ----------------------------------------------------------------------------


vrf_public vrf_private::to_public() const
{
    vrf_public pk;
    sodium::crypto_vrf_sk_to_pk(pk.data(), secret_.data());
    return pk;
}

std::string vrf_private::encoded() const
{
    return encode_base16(secret_);
}
// Operators.
// ----------------------------------------------------------------------------

vrf_private& vrf_private::operator=(const vrf_private& other)
{
    secret_ = other.secret_;
    return *this;
}

bool vrf_private::operator<(const vrf_private& other) const
{
    return encoded() < other.encoded();
}

bool vrf_private::operator==(const vrf_private& other) const
{
    return secret_ == other.secret_;
}

bool vrf_private::operator!=(const vrf_private& other) const
{
    return !(*this == other);
}

// We must assume mainnet for public version here.
// When converting this to public a clone of this key should be used, with the
// public version specified - after validating the private version.
std::istream& operator>>(std::istream& in, vrf_private& to)
{
    std::string value;
    in >> value;
    data_chunk data;
    decode_base16(data, value);
    to = vrf_private(data);
    return in;
}

std::ostream& operator<<(std::ostream& out, const vrf_private& of)
{
    out << of.encoded();
    return out;
}

bool vrf_private::prove(vrf_proof& proof, const data_chunk & m)
{
    return sodium::crypto_vrf_prove(proof.data(), secret_.data(), m.data(), m.size()) == 0;
}
bool vrf_private::proof_to_hash(vrf_hash& result, const vrf_proof& proof)
{
    return sodium::crypto_vrf_proof_to_hash(result.data(), proof.data()) == 0;
}
bool vrf_private::verify(vrf_hash& result, const vrf_proof& proof, const vrf_public& pk, const data_chunk & m)
{
    return sodium::crypto_vrf_verify(result.data(), pk.data(), proof.data(), m.data(), m.size()) == 0;
}


} // namespace wallet
} // namespace libbitcoin
