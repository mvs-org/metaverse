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
#ifndef MVS_VRF_HPP
#define MVS_VRF_HPP

#include <cstddef>
#include <string>
#include <vector>
#include <metaverse/bitcoin/define.hpp>
#include "hash.hpp"
#include "elliptic_curve.hpp"

namespace libbitcoin {
BC_API
bool ecvrf_prove(data_chunk & proof, const data_chunk & msg, ec_secret& sc);

BC_API
bool ecvrf_proof_to_hash(data_chunk& result, const data_chunk& proof);

BC_API
bool ecvrf_verify(const data_chunk& result,const data_slice& pk, 
    const data_chunk & msg, const data_chunk& proof);

} // namespace libbitcoin


#endif
