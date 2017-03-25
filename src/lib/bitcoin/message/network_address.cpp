/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS)
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
#include <metaverse/bitcoin/message/network_address.hpp>

#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>
#include <string.h>

#define	INADDR_NONE		((in_addr_t) 0xffffffff)

namespace libbitcoin {
namespace message {

ip_address null_address =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const unsigned char pchIPv4[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff };
static const unsigned char pchOnionCat[] = {0xFD,0x87,0xD8,0x7E,0xEB,0x43};

network_address network_address::factory_from_data(uint32_t version,
    const data_chunk& data, bool with_timestamp)
{
    network_address instance;
    instance.from_data(version, data, with_timestamp);
    return instance;
}

network_address network_address::factory_from_data(uint32_t version,
    std::istream& stream, bool with_timestamp)
{
    network_address instance;
    instance.from_data(version, stream, with_timestamp);
    return instance;
}

network_address network_address::factory_from_data(uint32_t version,
    reader& source, bool with_timestamp)
{
    network_address instance;
    instance.from_data(version, source, with_timestamp);
    return instance;
}

bool network_address::is_valid() const
{
#if 0
    return (timestamp != 0)
        || (services != 0)
        || (port != 0)
        || (ip != null_address);
#else
    // Cleanup 3-byte shifted addresses caused by garbage in size field
	// of addr messages from versions before 0.2.9 checksum.
	// Two consecutive addr messages look like this:
	// header20 vectorlen3 addr26 addr26 addr26 header20 vectorlen3 addr26 addr26 addr26...
	// so if the first length field is garbled, it reads the second batch
	// of addr misaligned by 3 bytes.
	if (memcmp(ip.data(), pchIPv4+3, sizeof(pchIPv4)-3) == 0)
		return false;

	// unspecified IPv6 address (::/128)
	unsigned char ipNone[16] = {};
	if (memcmp(ip.data(), ipNone, 16) == 0)
		return false;

	// documentation IPv6 address
	if (is_RFC3849())
		return false;

	if (is_ipv4())
	{
		// INADDR_NONE
		uint32_t ipNone = 0;
		if (memcmp(&ip[12], &ipNone, 4) == 0)
			return false;

		// 0
		ipNone = 0;
		if (memcmp(&ip[12], &ipNone, 4) == 0)
			return false;
	}

	return true;
#endif
}

void network_address::reset()
{
    timestamp = 0;
    services = 0;
    ip.fill(0);
    port = 0;
}

bool network_address::from_data(uint32_t version,
    const data_chunk& data, bool with_timestamp)
{
    data_source istream(data);
    return from_data(version, istream, with_timestamp);
}

bool network_address::from_data(uint32_t version,
    std::istream& stream, bool with_timestamp)
{
    istream_reader source(stream);
    return from_data(version, source, with_timestamp);
}

bool network_address::from_data(uint32_t version,
    reader& source, bool with_timestamp)
{
    auto result = false;

    reset();

    if (with_timestamp)
        timestamp = source.read_4_bytes_little_endian();

    services = source.read_8_bytes_little_endian();
    const auto ip_size = source.read_data(ip.data(), ip.size());
    port = source.read_2_bytes_big_endian();
    result = source && (ip.size() == ip_size);

    if (!result)
        reset();

    return result;
}

data_chunk network_address::to_data(uint32_t version,
    bool with_timestamp) const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(version, ostream, with_timestamp);
    ostream.flush();
    BITCOIN_ASSERT(data.size() == serialized_size(version, with_timestamp));
    return data;
}

void network_address::to_data(uint32_t version,
    std::ostream& stream, bool with_timestamp) const
{
    ostream_writer sink(stream);
    to_data(version, sink, with_timestamp);
}

void network_address::to_data(uint32_t version,
    writer& sink, bool with_timestamp) const
{
    if (with_timestamp)
        sink.write_4_bytes_little_endian(timestamp);

    sink.write_8_bytes_little_endian(services);
    sink.write_data(ip.data(), ip.size());
    sink.write_2_bytes_big_endian(port);
}

uint64_t network_address::serialized_size(uint32_t version,
    bool with_timestamp) const
{
    return network_address::satoshi_fixed_size(version, with_timestamp);
}

uint64_t network_address::satoshi_fixed_size(uint32_t version,
    bool with_timestamp)
{
    uint64_t result = 26;

    if (with_timestamp)
        result += 4;

    return result;
}


unsigned int network_address::get_byte(int n) const
{
    return ip[15-n];
}

bool network_address::is_ipv4() const
{
    return (memcmp(ip.data(), pchIPv4, sizeof(pchIPv4)) == 0);
}

bool network_address::is_ipv6() const
{
    return (!is_ipv4() && !is_tor());
}

bool network_address::is_private_network()
{
	return is_RFC1918();
}

bool network_address::is_RFC1918() const
{
#if 0 //fixme jianglh
    return is_ipv4() && (
        get_byte(3) == 10 ||
        (get_byte(3) == 192 && get_byte(2) == 168) ||
        (get_byte(3) == 172 && (get_byte(2) >= 16 && get_byte(2) <= 31)));
#else
	return false;
#endif
}

bool network_address::is_RFC3927() const
{
    return is_ipv4() && (get_byte(3) == 169 && get_byte(2) == 254);
}

bool network_address::is_RFC3849() const
{
    return get_byte(15) == 0x20 && get_byte(14) == 0x01 && get_byte(13) == 0x0D && get_byte(12) == 0xB8;
}

bool network_address::is_RFC3964() const
{
    return (get_byte(15) == 0x20 && get_byte(14) == 0x02);
}

bool network_address::is_RFC6052() const
{
    static const unsigned char pchRFC6052[] = {0,0x64,0xFF,0x9B,0,0,0,0,0,0,0,0};
    return (memcmp(ip.data(), pchRFC6052, sizeof(pchRFC6052)) == 0);
}

bool network_address::is_RFC4380() const
{
    return (get_byte(15) == 0x20 && get_byte(14) == 0x01 && get_byte(13) == 0 && get_byte(12) == 0);
}

bool network_address::is_RFC4862() const
{
    static const unsigned char pchRFC4862[] = {0xFE,0x80,0,0,0,0,0,0};
    return (memcmp(ip.data(), pchRFC4862, sizeof(pchRFC4862)) == 0);
}

bool network_address::is_RFC4193() const
{
    return ((get_byte(15) & 0xFE) == 0xFC);
}

bool network_address::is_RFC6145() const
{
    static const unsigned char pchRFC6145[] = {0,0,0,0,0,0,0,0,0xFF,0xFF,0,0};
    return (memcmp(ip.data(), pchRFC6145, sizeof(pchRFC6145)) == 0);
}

bool network_address::is_RFC4843() const
{
    return (get_byte(15) == 0x20 && get_byte(14) == 0x01 && get_byte(13) == 0x00 && (get_byte(12) & 0xF0) == 0x10);
}

bool network_address::is_tor() const
{
    return (memcmp(ip.data(), pchOnionCat, sizeof(pchOnionCat)) == 0);
}

bool network_address::is_local() const
{
    // IPv4 loopback
   if (is_ipv4() && (get_byte(3) == 127 || get_byte(3) == 0))
       return true;

   // IPv6 loopback (::1/128)
   static const unsigned char pchLocal[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
   if (memcmp(ip.data(), pchLocal, 16) == 0)
       return true;

   return false;
}

bool network_address::is_routable() const
{
    return is_valid() && !(is_RFC1918() || is_RFC3927() || is_RFC4862() || (is_RFC4193() && !is_tor()) || is_RFC4843() || is_local());
}

bool network_address::is_ulticast() const
{
    return    (is_ipv4() && (get_byte(3) & 0xF0) == 0xE0)
           || (get_byte(15) == 0xFF);
}


} // namspace message
} // namspace libbitcoin
