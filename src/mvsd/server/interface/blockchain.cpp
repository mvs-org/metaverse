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
#include <metaverse/server/interface/blockchain.hpp>

#include <cstdint>
#include <cstddef>
#include <functional>
#include <metaverse/blockchain.hpp>
#include <metaverse/server/define.hpp>
#include <metaverse/server/messages/message.hpp>
#include <metaverse/server/server_node.hpp>
#include <metaverse/server/utility/fetch_helpers.hpp>

namespace libbitcoin {
namespace server {

using namespace std::placeholders;
using namespace bc::blockchain;
using namespace bc::chain;
using namespace bc::wallet;

void blockchain::fetch_history(server_node& node, const message& request,
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

    log::debug(LOG_SERVER)
        << "blockchain.fetch_history(" << address.encoded()
        << ", from_height=" << from_height << ")";

    node.chain().fetch_history(address, limit, from_height,
        std::bind(send_history_result,
            _1, _2, request, handler));
}

void blockchain::fetch_transaction(server_node& node, const message& request,
    send_handler handler)
{
    hash_digest tx_hash;

    if (!unwrap_fetch_transaction_args(tx_hash, request))
    {
        handler(message(request, error::bad_stream));
        return;
    }

    log::debug(LOG_SERVER)
        << "blockchain.fetch_transaction(" << encode_hash(tx_hash) << ")";

    node.chain().fetch_transaction(tx_hash,
        std::bind(chain_transaction_fetched,
            _1, _2, request, handler));
}

void blockchain::fetch_last_height(server_node& node, const message& request,
    send_handler handler)
{
    const auto& data = request.data();

    if (!data.empty())
    {
        handler(message(request, error::bad_stream));
        return;
    }

    node.chain().fetch_last_height(
        std::bind(&blockchain::last_height_fetched,
            _1, _2, request, handler));
}

void blockchain::last_height_fetched(const code& ec, size_t last_height,
    const message& request, send_handler handler)
{
    BITCOIN_ASSERT(last_height <= max_uint32);
    auto last_height32 = static_cast<uint32_t>(last_height);

    // [ code:4 ]
    // [ heigh:4 ]
    const auto result = build_chunk(
    {
        message::to_bytes(ec),
        to_little_endian(last_height32)
    });

    handler(message(request, result));
}

void blockchain::fetch_block_header(server_node& node, const message& request,
    send_handler handler)
{
    const auto& data = request.data();

    if (data.size() == hash_size)
        blockchain::fetch_block_header_by_hash(node, request, handler);
    else if (data.size() == sizeof(uint32_t))
        blockchain::fetch_block_header_by_height(node, request, handler);
    else
        handler(message(request, error::bad_stream));
}

void blockchain::fetch_block_header_by_hash(server_node& node,
    const message& request, send_handler handler)
{
    const auto& data = request.data();
    BITCOIN_ASSERT(data.size() == hash_size);

    auto deserial = make_deserializer(data.begin(), data.end());
    const auto block_hash = deserial.read_hash();

    node.chain().fetch_block_header(block_hash,
        std::bind(&blockchain::block_header_fetched,
            _1, _2, request, handler));
}

void blockchain::fetch_block_header_by_height(server_node& node,
    const message& request, send_handler handler)
{
    const auto& data = request.data();
    BITCOIN_ASSERT(data.size() == sizeof(uint32_t));

    auto deserial = make_deserializer(data.begin(), data.end());
    const uint64_t height = deserial.read_4_bytes_little_endian();

    node.chain().fetch_block_header(height,
        std::bind(&blockchain::block_header_fetched,
            _1, _2, request, handler));
}

void blockchain::block_header_fetched(const code& ec,
    const chain::header& block, const message& request, send_handler handler)
{
    // [ code:4 ]
    // [ block... ]
    const auto result = build_chunk(
    {
        message::to_bytes(ec),
        block.to_data(false)
    });

    handler(message(request, result));
}

void blockchain::fetch_block_transaction_hashes(server_node& node,
    const message& request, send_handler handler)
{
    const auto& data = request.data();

    if (data.size() == hash_size)
        fetch_block_transaction_hashes_by_hash(node, request, handler);
    else if (data.size() == sizeof(uint32_t))
        fetch_block_transaction_hashes_by_height(node, request, handler);
    else
        handler(message(request, error::bad_stream));
}

void blockchain::fetch_block_transaction_hashes_by_hash(server_node& node,
    const message& request, send_handler handler)
{
    const auto& data = request.data();
    BITCOIN_ASSERT(data.size() == hash_size);

    auto deserial = make_deserializer(data.begin(), data.end());
    const auto block_hash = deserial.read_hash();
    node.chain().fetch_block_transaction_hashes(block_hash,
        std::bind(&blockchain::block_transaction_hashes_fetched,
            _1, _2, request, handler));
}

void blockchain::fetch_block_transaction_hashes_by_height(server_node& node,
    const message& request, send_handler handler)
{
    const auto& data = request.data();
    BITCOIN_ASSERT(data.size() == sizeof(uint32_t));

    auto deserial = make_deserializer(data.begin(), data.end());
    const size_t block_height = deserial.read_4_bytes_little_endian();
    node.chain().fetch_block_transaction_hashes(block_height,
        std::bind(&blockchain::block_transaction_hashes_fetched,
            _1, _2, request, handler));
}

void blockchain::block_transaction_hashes_fetched(const code& ec,
    const hash_list& hashes, const message& request, send_handler handler)
{
    // [ code:4 ]
    // [[ hash:32 ]...]
    data_chunk result(code_size + hash_size * hashes.size());
    auto serial = make_serializer(result.begin());
    serial.write_error_code(ec);

    for (const auto& tx_hash: hashes)
        serial.write_hash(tx_hash);

    handler(message(request, result));
}

void blockchain::fetch_transaction_index(server_node& node,
    const message& request, send_handler handler)
{
    const auto& data = request.data();

    if (data.size() != hash_size)
    {
        handler(message(request, error::bad_stream));
        return;
    }

    auto deserial = make_deserializer(data.begin(), data.end());
    const auto tx_hash = deserial.read_hash();

    node.chain().fetch_transaction_index(tx_hash,
        std::bind(&blockchain::transaction_index_fetched,
            _1, _2, _3, request, handler));
}

void blockchain::transaction_index_fetched(const code& ec, size_t block_height,
    size_t index, const message& request, send_handler handler)
{
    BITCOIN_ASSERT(index <= max_uint32);
    BITCOIN_ASSERT(block_height <= max_uint32);

    auto index32 = static_cast<uint32_t>(index);
    auto block_height32 = static_cast<uint32_t>(block_height);

    // [ code:4 ]
    // [ block_height:32 ]
    // [ tx_index:4 ]
    const auto result = build_chunk(
    {
        message::to_bytes(ec),
        to_little_endian(block_height32),
        to_little_endian(index32)
    });

    handler(message(request, result));
}

void blockchain::fetch_spend(server_node& node, const message& request,
    send_handler handler)
{
    const auto& data = request.data();

    if (data.size() != point_size)
    {
        handler(message(request, error::bad_stream));
        return;
    }

    using namespace boost::iostreams;
    static const auto fail_bit = stream<byte_source<data_chunk>>::failbit;
    stream<byte_source<data_chunk>> istream(data);
    istream.exceptions(fail_bit);
    chain::output_point outpoint;
    outpoint.from_data(istream);

    node.chain().fetch_spend(outpoint,
        std::bind(&blockchain::spend_fetched,
            _1, _2, request, handler));
}

void blockchain::spend_fetched(const code& ec, const chain::input_point& inpoint,
    const message& request, send_handler handler)
{
    // [ code:4 ]
    // [ hash:32 ]
    // [ index:4 ]
    const auto result = build_chunk(
    {
        message::to_bytes(ec),
        inpoint.to_data()
    });

    handler(message(request, result));
}

void blockchain::fetch_block_height(server_node& node,
    const message& request, send_handler handler)
{
    const auto& data = request.data();

    if (data.size() != hash_size)
    {
        handler(message(request, error::bad_stream));
        return;
    }

    auto deserial = make_deserializer(data.begin(), data.end());
    const auto block_hash = deserial.read_hash();
    node.chain().fetch_block_height(block_hash,
        std::bind(&blockchain::block_height_fetched,
            _1, _2, request, handler));
}

void blockchain::block_height_fetched(const code& ec, size_t block_height,
    const message& request, send_handler handler)
{
    BITCOIN_ASSERT(block_height <= max_uint32);
    auto block_height32 = static_cast<uint32_t>(block_height);

    // [ code:4 ]
    // [ height:4 ]
    const auto result = build_chunk(
    {
        message::to_bytes(ec),
        to_little_endian(block_height32)
    });

    handler(message(request, result));
}

void blockchain::fetch_stealth(server_node& node, const message& request,
    send_handler handler)
{
    const auto& data = request.data();

    if (data.empty())
    {
        handler(message(request, error::bad_stream));
        return;
    }

    auto deserial = make_deserializer(data.begin(), data.end());

    // number_bits
    const auto bit_size = deserial.read_byte();

    if (data.size() != sizeof(uint8_t) + binary::blocks_size(bit_size) +
        sizeof(uint32_t))
    {
        handler(message(request, error::bad_stream));
        return;
    }

    // actual bitfield data
    const auto blocks = deserial.read_data(binary::blocks_size(bit_size));
    const binary prefix(bit_size, blocks);

    // from_height
    const uint64_t from_height = deserial.read_4_bytes_little_endian();

    node.chain().fetch_stealth(prefix, from_height,
        std::bind(&blockchain::stealth_fetched,
            _1, _2, request, handler));
}

void blockchain::stealth_fetched(const code& ec,
    const stealth_compact::list& stealth_results, const message& request,
    send_handler handler)
{
    static constexpr size_t row_size = hash_size + short_hash_size + hash_size;

    // [ code:4 ]
    // [[ ephemeral_key_hash:32 ][ address_hash:20 ][ tx_hash:32 ]...]
    data_chunk result(code_size + row_size * stealth_results.size());
    auto serial = make_serializer(result.begin());
    serial.write_error_code(ec);

    for (const auto& row: stealth_results)
    {
        serial.write_hash(row.ephemeral_public_key_hash);
        serial.write_short_hash(row.public_key_hash);
        serial.write_hash(row.transaction_hash);
    }

    handler(message(request, result));
}

void blockchain::fetch_stealth2(server_node& node, const message& request,
    send_handler handler)
{
    const auto& data = request.data();

    if (data.empty())
    {
        handler(message(request, error::bad_stream));
        return;
    }

    auto deserial = make_deserializer(data.begin(), data.end());

    // number_bits
    const auto bit_size = deserial.read_byte();

    if (data.size() != sizeof(uint8_t) + binary::blocks_size(bit_size) +
        sizeof(uint32_t))
    {
        handler(message(request, error::bad_stream));
        return;
    }

    // actual bitfield data
    const auto blocks = deserial.read_data(binary::blocks_size(bit_size));
    const binary prefix(bit_size, blocks);

    // from_height
    const uint64_t from_height = deserial.read_4_bytes_little_endian();

    node.chain().fetch_stealth(prefix, from_height,
        std::bind(&blockchain::stealth_fetched2,
            _1, _2, request, handler));
}

void blockchain::stealth_fetched2(const code& ec,
    const stealth_compact::list& stealth_results, const message& request,
    send_handler handler)
{
    // [ code:4 ]
    // [[ tx_hash:32 ]...]
    data_chunk result(code_size + hash_size * stealth_results.size());
    auto serial = make_serializer(result.begin());
    serial.write_error_code(ec);

    for (const auto& row: stealth_results)
        serial.write_hash(row.transaction_hash);

    handler(message(request, result));
}

} // namespace server
} // namespace libbitcoin
