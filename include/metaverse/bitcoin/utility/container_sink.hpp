/**
 * Copyright (c) 2011-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2020 metaverse core developers (see MVS-AUTHORS)
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
#ifndef MVS_CONTAINER_SINK_HPP
#define MVS_CONTAINER_SINK_HPP

#include <algorithm>
#include <cstdint>
#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/define.hpp>

namespace libbitcoin {

// modified from boost.iostreams example
// boost.org/doc/libs/1_55_0/libs/iostreams/doc/tutorial/container_source.html
template <typename Container, typename SinkType, typename CharType>
class BC_API container_sink
{
public:
    typedef CharType char_type;
    typedef boost::iostreams::sink_tag category;

    container_sink(Container& container)
      : container_(container)
    {
        static_assert(sizeof(SinkType) == sizeof(CharType), "invalid size");
    }

    std::streamsize write(const char_type* buffer, std::streamsize size)
    {
        const auto safe_sink = reinterpret_cast<const SinkType*>(buffer);
        container_.insert(container_.end(), safe_sink, safe_sink + size);
        return size;
    }

private:
    Container& container_;
};

template <typename Container>
using byte_sink = container_sink<Container, uint8_t, char>;

using data_sink = boost::iostreams::stream<byte_sink<data_chunk>>;

} // namespace libbitcoin

#endif

