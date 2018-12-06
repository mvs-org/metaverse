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
#include <metaverse/bitcoin/math/hash.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <errno.h>
#include <new>
#include <stdexcept>
#include <metaverse/bitcoin/math/ecvrf.hpp>
#include "external/sha512.h"

namespace libbitcoin {
    
bool ecvrf_prove(data_chunk & proof, const data_chunk & msg, ec_secret& sc)
{
    if (msg.size() == 0) {
        return false;
    }
    const auto bytes = sha256_hash(msg);
    bc::ec_signature signature;
    endorsement out;
    if (!sign(signature, sc, bytes) || !encode_signature(proof, signature)) {
        return false;
    }

    return true;
}

    
bool ecvrf_proof_to_hash(data_chunk& result, const data_chunk& proof)
{
    if (proof.size() == 0) {
        return false;
    }

    const auto bytes = sha256_hash(proof);
    std::copy(bytes.begin(), bytes.end(), std::back_inserter(result));

    return true;
}

    
bool ecvrf_verify(const data_chunk& result, const data_slice& pk, const data_chunk & msg, const data_chunk& proof)
{
    if (!is_public_key(pk) || msg.size() == 0) {
        return false;
    }

    const auto bytes = sha256_hash(msg);
    ec_signature signature;
    if (!parse_signature(signature, proof, false) || !verify_signature(pk, bytes, signature)) {
        return false;
    }

    data_chunk result1;
    if (!ecvrf_proof_to_hash(result1, proof)) {
        return false;
    }

    return result == result1;
}
} // namespace libbitcoin
