/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
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
#include <bitcoin/node/protocols/protocol_transaction_out.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <bitcoin/network.hpp>

namespace libbitcoin {
namespace node {

#define NAME "transaction"
#define CLASS protocol_transaction_out
    
using namespace bc::blockchain;
using namespace bc::message;
using namespace bc::network;
using namespace std::placeholders;

protocol_transaction_out::protocol_transaction_out(p2p& network,
    channel::ptr channel, block_chain& blockchain, transaction_pool& pool)
  : protocol_events(network, channel, NAME),
    blockchain_(blockchain),
    pool_(pool),

    // TODO: move fee filter to a derived class protocol_transaction_out_70013.
    minimum_fee_(0),

    // TODO: move relay to a derived class protocol_transaction_out_70001.
    relay_to_peer_(peer_version().relay),
    CONSTRUCT_TRACK(protocol_transaction_out)
{
}

// TODO: move not_found to derived class protocol_transaction_out_70001.

// Start.
//-----------------------------------------------------------------------------

void protocol_transaction_out::start()
{
    // TODO: move relay to a derived class protocol_transaction_out_70001.
    // Prior to this level transaction relay is not configurable.
    if (relay_to_peer_)
    {
        // Subscribe to transaction pool notifications and relay txs.
        pool_.subscribe_transaction(BIND3(handle_floated, _1, _2, _3));
    }

    // TODO: move fee filter to a derived class protocol_transaction_out_70013.
    // Filter announcements by fee if set.
    SUBSCRIBE2(fee_filter, handle_receive_fee_filter, _1, _2);
}

// Receive send_headers.
//-----------------------------------------------------------------------------

// TODO: move fee_filters to a derived class protocol_transaction_out_70013.
bool protocol_transaction_out::handle_receive_fee_filter(const code& ec,
    fee_filter_ptr message)
{
    if (stopped())
        return false;

    if (ec)
    {
        log::debug(LOG_NODE)
            << "Failure getting " << message->command << " from ["
            << authority() << "] " << ec.message();
        stop(ec);
        return false;
    }

    // TODO: move fee filter to a derived class protocol_transaction_out_70013.
    // Transaction annoucements will be filtered by fee amount.
    minimum_fee_.store(message->minimum_fee);

    // The fee filter may be adjusted.
    return true;
}

// Receive mempool sequence.
//-----------------------------------------------------------------------------

void protocol_transaction_out::handle_receive_memory_pool(const code& ec,
    memory_pool_ptr)
{
    auto message = std::make_shared<inventory>();
    pool_.inventory(message);
    SEND2(*message, handle_send, _1, message->command);
}

// Receive get_data sequence.
//-----------------------------------------------------------------------------

bool protocol_transaction_out::handle_receive_get_data(const code& ec,
    get_data_ptr message)
{
    if (stopped())
        return false;

    if (ec)
    {
        log::debug(LOG_NODE)
            << "Failure getting inventory from [" << authority() << "] "
            << ec.message();
        stop(ec);
        return false;
    }

    // TODO: these must return message objects or be copied!
    // Ignore non-transaction inventory requests in this protocol.
    for (const auto& inventory: message->inventories)
        if (inventory.type == inventory::type_id::transaction)
            blockchain_.fetch_transaction(inventory.hash,
                BIND3(send_transaction, _1, _2, inventory.hash));

    return true;
}

void protocol_transaction_out::send_transaction(const code& ec,
    const chain::transaction& transaction, const hash_digest& hash)
{
    if (stopped() || ec == error::service_stopped)
        return;

    if (ec == error::not_found)
    {
        log::debug(LOG_NODE)
            << "Transaction requested by [" << authority() << "] not found.";

        const not_found reply{ { inventory::type_id::transaction, hash } };
        SEND2(reply, handle_send, _1, reply.command);
        return;
    }

    if (ec)
    {
        log::error(LOG_NODE)
            << "Internal failure locating trnsaction requested by ["
            << authority() << "] " << ec.message();
        stop(ec);
        return;
    }

    // TODO: eliminate copy.
    SEND2(transaction_message(transaction), handle_send, _1,
        transaction_message::command);
}

// Subscription.
//-----------------------------------------------------------------------------

bool protocol_transaction_out::handle_floated(const code& ec,
    const index_list& unconfirmed, transaction_ptr message)
{
    if (stopped() || ec == error::service_stopped)
        return false;

    if (ec)
    {
        log::error(LOG_NODE)
            << "Failure handling transaction float: " << ec.message();
        stop(ec);
        return false;
    }

    // TODO: move fee filter to a derived class protocol_transaction_out_70013.
    // TODO: implement fee computation.
    const uint64_t fee = 0;

    // Transactions are discovered and announced individually.
    if (message->originator() != nonce() && fee >= minimum_fee_.load())
    {
        static const auto id = inventory::type_id::transaction;
        const inventory announcement{ { id, message->hash() } };
        SEND2(announcement, handle_send, _1, announcement.command);
    }

    return true;
}

void protocol_transaction_out::handle_stop(const code&)
{
    log::debug(LOG_NETWORK)
        << "Stopped transaction_out protocol";
}

} // namespace node
} // namespace libbitcoin
