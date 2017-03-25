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
#include <metaverse/server/interface/address.hpp>

#include <cstdint>
#include <functional>
#include <metaverse/bitcoin.hpp>
#include <metaverse/server/messages/message.hpp>
#include <metaverse/server/server_node.hpp>
#include <metaverse/server/utility/fetch_helpers.hpp>

namespace libbitcoin {
namespace server {

using namespace std::placeholders;
using namespace bc::chain;
using namespace bc::wallet;

void address::fetch_history2(server_node& node, const message& request,
    send_handler handler)
{
    static constexpr uint64_t limit = 0;
    uint32_t from_height;
    payment_address address;

    if (!unwrap_fetch_history_args(address, from_height, request))
    {
        handler(message(request, error::bad_stream));
        return;
    }

    // Obtain payment address history from the transaction pool and blockchain.
    node.pool().fetch_history(address, limit, from_height,
        std::bind(send_history_result,
            _1, _2, request, handler));
}

// v2/v3 (deprecated), used for resubscription, alias for subscribe in v3.
void address::renew(server_node& node, const message& request,
    send_handler handler)
{
    subscribe(node, request, handler);
}

// v2/v3 (deprecated), requires an explicit subscription type.
void address::subscribe(server_node& node, const message& request,
    send_handler handler)
{
    binary prefix_filter;
    subscribe_type type;

    if (!unwrap_subscribe_args(prefix_filter, type, request))
    {
        handler(message(request, error::bad_stream));
        return;
    }

    node.subscribe_address(request.route(), request.id(), prefix_filter, type);
    handler(message(request, error::success));
}

bool address::unwrap_subscribe_args(binary& prefix_filter,
    subscribe_type& type, const message& request)
{
    static constexpr auto address_bits = short_hash_size * byte_bits;
    static constexpr auto stealth_bits = sizeof(uint32_t) * byte_bits;

    // [ type:1 ] (0 = address prefix, 1 = stealth prefix)
    // [ prefix_bitsize:1 ]
    // [ prefix_blocks:...]
    const auto& data = request.data();

    if (data.size() < 2)
        return false;

    // First byte is the subscribe_type enumeration.
    type = static_cast<subscribe_type>(data[0]);

    if (type != subscribe_type::payment && type != subscribe_type::stealth)
        return false;

    // Second byte is the number of bits.
    const auto bit_length = data[1];

    if ((type == subscribe_type::payment && bit_length > address_bits) ||
        (type == subscribe_type::stealth && bit_length > stealth_bits))
        return false;

    // Convert the bit length to byte length.
    const auto byte_length = binary::blocks_size(bit_length);

    if (data.size() - 2 != byte_length)
        return false;

    const data_chunk bytes({ data.begin() + 2, data.end() });
    prefix_filter = binary(bit_length, bytes);
    return true;
}

// v3 eliminates the subscription type, which we map to 'unspecified'.
void address::subscribe2(server_node& node, const message& request,
    send_handler handler)
{
    static constexpr auto type = subscribe_type::unspecified;

    binary prefix_filter;

    if (!unwrap_subscribe2_args(prefix_filter, request))
    {
        handler(message(request, error::bad_stream));
        return;
    }

    node.subscribe_address(request.route(), request.id(), prefix_filter, type);
    handler(message(request, error::success));
}

// v3 adds unsubscribe2, which we map to subscription_type 'unsubscribe'.
void address::unsubscribe2(server_node& node, const message& request,
    send_handler handler)
{
    static constexpr auto type = subscribe_type::unsubscribe;

    binary prefix_filter;

    if (!unwrap_subscribe2_args(prefix_filter, request))
    {
        handler(message(request, error::bad_stream));
        return;
    }

    node.subscribe_address(request.route(), request.id(), prefix_filter, type);
    handler(message(request, error::success));
}

bool address::unwrap_subscribe2_args(binary& prefix_filter,
    const message& request)
{
    static constexpr auto address_bits = hash_size * byte_bits;

    // [ prefix_bitsize:1 ]
    // [ prefix_blocks:...]
    const auto& data = request.data();

    if (data.empty())
        return false;

    // First byte is the number of bits.
    const auto bit_length = data[0];

    // always false for this case: chenhao
    //if (bit_length > address_bits)
    //    return false;

    // Convert the bit length to byte length.
    const auto byte_length = binary::blocks_size(bit_length);

    if (data.size() - 1 != byte_length)
        return false;

    const data_chunk bytes({ data.begin() + 1, data.end() });
    prefix_filter = binary(bit_length, bytes);
    return true;
}

} // namespace server
} // namespace libbitcoin
