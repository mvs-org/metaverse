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
#ifndef MVS_MESSAGE_NETWORK_ADDRESS_HPP
#define MVS_MESSAGE_NETWORK_ADDRESS_HPP

#include <cstdint>
#include <istream>
#include <vector>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/utility/data.hpp>
#include <metaverse/bitcoin/utility/reader.hpp>
#include <metaverse/bitcoin/utility/writer.hpp>

namespace libbitcoin {
namespace message {

typedef byte_array<16> ip_address;

BC_CONSTEXPR ip_address localhost_ip_address =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xff, 0xff, 0x0a, 0x00, 0x00, 0x01
};

class BC_API network_address
{
public:
    typedef std::vector<network_address> list;

    static network_address factory_from_data(uint32_t version,
        const data_chunk& data, bool with_timestamp /*= true*/);
    static network_address factory_from_data(uint32_t version,
        std::istream& stream, bool with_timestamp /*= true*/);
    static network_address factory_from_data(uint32_t version,
        reader& source, bool with_timestamp /*=true*/);
    static uint64_t satoshi_fixed_size(uint32_t version,
        bool with_timestamp /*= false*/);

    bool from_data(uint32_t version, const data_chunk& data,
        bool with_timestamp /*= true*/);
    bool from_data(uint32_t version, std::istream& stream,
        bool with_timestamp /*= true*/);
    bool from_data(uint32_t version, reader& source,
        bool with_timestamp /*= true*/);
    data_chunk to_data(uint32_t version,
        bool with_timestamp /*= true*/) const;
    void to_data(uint32_t version, std::ostream& stream,
        bool with_timestamp /*= true*/) const;
    void to_data(uint32_t version, writer& sink,
        bool with_timestamp /*= true*/) const;
    bool is_valid() const;

    unsigned int get_byte(int n) const;
    bool is_ipv4() const;    // IPv4 mapped address (::FFFF:0:0/96, 0.0.0.0/0)
       bool is_ipv6() const;    // IPv6 address (not mapped IPv4, not Tor)
       bool is_private_network();
       bool is_RFC1918() const; // IPv4 private networks (10.0.0.0/8, 192.168.0.0/16, 172.16.0.0/12)
       bool is_RFC3849() const; // IPv6 documentation address (2001:0DB8::/32)
       bool is_RFC3927() const; // IPv4 autoconfig (169.254.0.0/16)
       bool is_RFC3964() const; // IPv6 6to4 tunnelling (2002::/16)
       bool is_RFC4193() const; // IPv6 unique local (FC00::/15)
       bool is_RFC4380() const; // IPv6 Teredo tunnelling (2001::/32)
       bool is_RFC4843() const; // IPv6 ORCHID (2001:10::/28)
       bool is_RFC4862() const; // IPv6 autoconfig (FE80::/64)
       bool is_RFC6052() const; // IPv6 well-known prefix (64:FF9B::/96)
       bool is_RFC6145() const; // IPv6 IPv4-translated address (::FFFF:0:0:0/96)
       bool is_tor() const;
       bool is_local() const;
       bool is_routable() const;
       bool is_ulticast() const;

    void reset();
    uint64_t serialized_size(uint32_t version,
        bool with_timestamp /*= false*/) const;

    // Starting version 31402, addresses are prefixed with a timestamp.
    uint32_t timestamp;
    uint64_t services;
    ip_address ip;
    uint16_t port;
};

} // namespace message
} // namespace libbitcoin

#endif
