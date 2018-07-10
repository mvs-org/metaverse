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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/network/message_subscriber.hpp>

#include <istream>
#include <memory>
#include <string>
#include <metaverse/bitcoin.hpp>

#define INITIALIZE_SUBSCRIBER(pool, value) \
    value##_subscriber_(std::make_shared<value##_subscriber_type>( \
        pool, #value "_sub"))

#define RELAY_CODE(code, value) \
    value##_subscriber_->relay(code, nullptr)

#define CASE_HANDLE_MESSAGE(stream, version, value) \
    case message_type::value: \
        return handle<message::value>(stream, version, value##_subscriber_)

#define CASE_RELAY_MESSAGE(stream, version, value) \
    case message_type::value: \
        return relay<message::value>(stream, version, value##_subscriber_)

#define START_SUBSCRIBER(value) \
    value##_subscriber_->start()

#define STOP_SUBSCRIBER(value) \
    value##_subscriber_->stop()

namespace libbitcoin {
namespace network {

using namespace message;

message_subscriber::message_subscriber(threadpool& pool)
  : INITIALIZE_SUBSCRIBER(pool, address),
    INITIALIZE_SUBSCRIBER(pool, block_message),
    INITIALIZE_SUBSCRIBER(pool, block_transactions),
    INITIALIZE_SUBSCRIBER(pool, compact_block),
    INITIALIZE_SUBSCRIBER(pool, fee_filter),
    INITIALIZE_SUBSCRIBER(pool, filter_add),
    INITIALIZE_SUBSCRIBER(pool, filter_clear),
    INITIALIZE_SUBSCRIBER(pool, filter_load),
    INITIALIZE_SUBSCRIBER(pool, get_address),
    INITIALIZE_SUBSCRIBER(pool, get_blocks),
    INITIALIZE_SUBSCRIBER(pool, get_block_transactions),
    INITIALIZE_SUBSCRIBER(pool, get_data),
    INITIALIZE_SUBSCRIBER(pool, get_headers),
    INITIALIZE_SUBSCRIBER(pool, headers),
    INITIALIZE_SUBSCRIBER(pool, inventory),
    INITIALIZE_SUBSCRIBER(pool, memory_pool),
    INITIALIZE_SUBSCRIBER(pool, merkle_block),
    INITIALIZE_SUBSCRIBER(pool, not_found),
    INITIALIZE_SUBSCRIBER(pool, ping),
    INITIALIZE_SUBSCRIBER(pool, pong),
    INITIALIZE_SUBSCRIBER(pool, reject),
    INITIALIZE_SUBSCRIBER(pool, send_headers),
    INITIALIZE_SUBSCRIBER(pool, send_compact_blocks),
    INITIALIZE_SUBSCRIBER(pool, transaction_message),
    INITIALIZE_SUBSCRIBER(pool, verack),
    INITIALIZE_SUBSCRIBER(pool, version)
{
}

void message_subscriber::broadcast(const code& ec)
{
    RELAY_CODE(ec, address);
    RELAY_CODE(ec, block_message);
    RELAY_CODE(ec, block_transactions);
    RELAY_CODE(ec, compact_block);
    RELAY_CODE(ec, fee_filter);
    RELAY_CODE(ec, filter_add);
    RELAY_CODE(ec, filter_clear);
    RELAY_CODE(ec, filter_load);
    RELAY_CODE(ec, get_address);
    RELAY_CODE(ec, get_blocks);
    RELAY_CODE(ec, get_block_transactions);
    RELAY_CODE(ec, get_data);
    RELAY_CODE(ec, get_headers);
    RELAY_CODE(ec, headers);
    RELAY_CODE(ec, inventory);
    RELAY_CODE(ec, memory_pool);
    RELAY_CODE(ec, merkle_block);
    RELAY_CODE(ec, not_found);
    RELAY_CODE(ec, ping);
    RELAY_CODE(ec, pong);
    RELAY_CODE(ec, reject);
    RELAY_CODE(ec, send_headers);
    RELAY_CODE(ec, send_compact_blocks);
    RELAY_CODE(ec, transaction_message);
    RELAY_CODE(ec, verack);
    RELAY_CODE(ec, version);
}

code message_subscriber::load(message_type type, uint32_t version,
    std::istream& stream) const
{
    switch (type)
    {
        CASE_RELAY_MESSAGE(stream, version, address);
        CASE_HANDLE_MESSAGE(stream, version, block_message);
        CASE_RELAY_MESSAGE(stream, version, block_transactions);
        CASE_RELAY_MESSAGE(stream, version, compact_block);
        CASE_RELAY_MESSAGE(stream, version, fee_filter);
        CASE_RELAY_MESSAGE(stream, version, filter_add);
        CASE_RELAY_MESSAGE(stream, version, filter_clear);
        CASE_RELAY_MESSAGE(stream, version, filter_load);
        CASE_RELAY_MESSAGE(stream, version, get_address);
        CASE_RELAY_MESSAGE(stream, version, get_blocks);
        CASE_RELAY_MESSAGE(stream, version, get_block_transactions);
        CASE_RELAY_MESSAGE(stream, version, get_data);
        CASE_RELAY_MESSAGE(stream, version, get_headers);
        CASE_RELAY_MESSAGE(stream, version, headers);
        CASE_RELAY_MESSAGE(stream, version, inventory);
        CASE_RELAY_MESSAGE(stream, version, memory_pool);
        CASE_RELAY_MESSAGE(stream, version, merkle_block);
        CASE_RELAY_MESSAGE(stream, version, not_found);
        CASE_RELAY_MESSAGE(stream, version, ping);
        CASE_RELAY_MESSAGE(stream, version, pong);
        CASE_RELAY_MESSAGE(stream, version, reject);
        CASE_RELAY_MESSAGE(stream, version, send_headers);
        CASE_RELAY_MESSAGE(stream, version, send_compact_blocks);
        CASE_RELAY_MESSAGE(stream, version, transaction_message);
        CASE_RELAY_MESSAGE(stream, version, verack);
        CASE_HANDLE_MESSAGE(stream, version, version);
        case message_type::unknown:
        default:
            return error::not_found;
    }
}

void message_subscriber::start()
{
    START_SUBSCRIBER(address);
    START_SUBSCRIBER(block_message);
    START_SUBSCRIBER(block_transactions);
    START_SUBSCRIBER(compact_block);
    START_SUBSCRIBER(fee_filter);
    START_SUBSCRIBER(filter_add);
    START_SUBSCRIBER(filter_clear);
    START_SUBSCRIBER(filter_load);
    START_SUBSCRIBER(get_address);
    START_SUBSCRIBER(get_blocks);
    START_SUBSCRIBER(get_block_transactions);
    START_SUBSCRIBER(get_data);
    START_SUBSCRIBER(get_headers);
    START_SUBSCRIBER(headers);
    START_SUBSCRIBER(inventory);
    START_SUBSCRIBER(memory_pool);
    START_SUBSCRIBER(merkle_block);
    START_SUBSCRIBER(not_found);
    START_SUBSCRIBER(ping);
    START_SUBSCRIBER(pong);
    START_SUBSCRIBER(reject);
    START_SUBSCRIBER(send_headers);
    START_SUBSCRIBER(send_compact_blocks);
    START_SUBSCRIBER(transaction_message);
    START_SUBSCRIBER(verack);
    START_SUBSCRIBER(version);
}

void message_subscriber::stop()
{
    STOP_SUBSCRIBER(address);
    STOP_SUBSCRIBER(block_message);
    STOP_SUBSCRIBER(block_transactions);
    STOP_SUBSCRIBER(compact_block);
    STOP_SUBSCRIBER(fee_filter);
    STOP_SUBSCRIBER(filter_add);
    STOP_SUBSCRIBER(filter_clear);
    STOP_SUBSCRIBER(filter_load);
    STOP_SUBSCRIBER(get_address);
    STOP_SUBSCRIBER(get_blocks);
    STOP_SUBSCRIBER(get_block_transactions);
    STOP_SUBSCRIBER(get_data);
    STOP_SUBSCRIBER(get_headers);
    STOP_SUBSCRIBER(headers);
    STOP_SUBSCRIBER(inventory);
    STOP_SUBSCRIBER(memory_pool);
    STOP_SUBSCRIBER(merkle_block);
    STOP_SUBSCRIBER(not_found);
    STOP_SUBSCRIBER(ping);
    STOP_SUBSCRIBER(pong);
    STOP_SUBSCRIBER(reject);
    STOP_SUBSCRIBER(send_headers);
    STOP_SUBSCRIBER(send_compact_blocks);
    STOP_SUBSCRIBER(transaction_message);
    STOP_SUBSCRIBER(verack);
    STOP_SUBSCRIBER(version);
}

} // namespace network
} // namespace libbitcoin
