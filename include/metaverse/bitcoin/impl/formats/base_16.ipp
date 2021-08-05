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
#ifndef MVS_BASE_16_IPP
#define MVS_BASE_16_IPP

#include <metaverse/bitcoin/utility/assert.hpp>

namespace libbitcoin {

// For template implementation only, do not call directly.
BC_API bool decode_base16_private(uint8_t* out, size_t out_size,
    const char* in);

template <size_t Size>
bool decode_base16(byte_array<Size>& out, const std::string &in)
{
    if (in.size() != 2 * Size)
        return false;

    byte_array<Size> result;
    if (!decode_base16_private(result.data(), result.size(), in.data()))
        return false;

    out = result;
    return true;
}

template <size_t Size>
byte_array<(Size - 1) / 2> base16_literal(const char (&string)[Size])
{
    byte_array<(Size - 1) / 2> out;
    DEBUG_ONLY(const auto success =) decode_base16_private(out.data(),
        out.size(), string);
    BITCOIN_ASSERT(success);
    return out;
}

} // libbitcoin

#endif
