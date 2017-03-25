/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse-server.
 *
 * metaverse-server is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
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
#ifndef MVS_SERVER_ADDRESS_KEY_HPP
#define MVS_SERVER_ADDRESS_KEY_HPP

#include <cstddef>
#include <boost/functional/hash_fwd.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/server/messages/route.hpp>
#include <metaverse/server/define.hpp>

namespace libbitcoin {
namespace server {

class BCS_API address_key
{
public:
    address_key(const route& reply_to, const binary& prefix_filter);
    bool operator==(const address_key& other) const;
    const route& reply_to() const;
    const binary& prefix_filter() const;

private:
    const route& reply_to_;
    const binary& prefix_filter_;
};

} // namespace server
} // namespace libbitcoin

namespace std
{
    template<>
    struct hash<bc::server::address_key>
    {
        size_t operator()(const bc::server::address_key& value) const
        {
            // boost::hash_combine uses boost::hash declarations., but these
            // are defined as std::hash (for use with std::map). So we must
            // explicity perform the hash operation before combining.
            const auto to = std::hash<bc::server::route>()(value.reply_to());
            const auto filter = std::hash<bc::binary>()(value.prefix_filter());

            size_t seed = 0;
            boost::hash_combine(seed, to);
            boost::hash_combine(seed, filter);
            return seed;
        }
    };
} // namespace std

#endif
