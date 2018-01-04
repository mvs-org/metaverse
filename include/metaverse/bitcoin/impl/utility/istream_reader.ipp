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
#ifndef MVS_ISTREAM_READER_IPP
#define MVS_ISTREAM_READER_IPP

#include <algorithm>
#include <boost/asio/streambuf.hpp>
#include <metaverse/bitcoin/utility/assert.hpp>
#include <metaverse/bitcoin/utility/endian.hpp>
#include <metaverse/bitcoin/utility/exceptions.hpp>

namespace libbitcoin {

template <typename T>
T istream_reader::read_big_endian()
{
    return from_big_endian_stream_unsafe<T>(stream_);
}

template <typename T>
T istream_reader::read_little_endian()
{
    return from_little_endian_stream_unsafe<T>(stream_);
}

template <unsigned Size>
byte_array<Size> istream_reader::read_bytes()
{
    byte_array<Size> out;

    for (unsigned i = 0; i < Size; i++)
        out[i] = read_byte();

    return out;
}

template <unsigned Size>
byte_array<Size> istream_reader::read_bytes_reverse()
{
    byte_array<Size> out;

    for (unsigned i = 0; i < Size; i++)
        out[Size - (i + 1)] = read_byte();

    return out;
}

} // libbitcoin

#endif
