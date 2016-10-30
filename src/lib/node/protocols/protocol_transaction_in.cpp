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
#include <bitcoin/node/protocols/protocol_transaction_in.hpp>

#include <cstddef>
#include <functional>
#include <memory>
#include <bitcoin/network.hpp>

namespace libbitcoin {
namespace node {

#define NAME "transaction"
#define CLASS protocol_transaction_in

using namespace bc::blockchain;
using namespace bc::message;
using namespace bc::network;
using namespace std::placeholders;

// TODO: derive from protocol_session_node abstract intermediate base class.
// TODO: Pass p2p_node on construct, obtaining node configuration settings.
protocol_transaction_in::protocol_transaction_in(p2p& network,
    channel::ptr channel, block_chain& blockchain, transaction_pool& pool)
  : protocol_events(network, channel, NAME),
    blockchain_(blockchain),
    pool_(pool),

    // TODO: move relay to a derived class protocol_transaction_in_70001.
    relay_from_peer_(network.network_settings().relay_transactions),

    // TODO: move memory_pool to a derived class protocol_transaction_in_60002.
    peer_suports_memory_pool_(peer_version().value >= version::level::bip35),
    refresh_pool_(relay_from_peer_ && peer_suports_memory_pool_
        /*&& network.node_settings().transaction_pool_refresh*/),

    CONSTRUCT_TRACK(protocol_transaction_in)
{
}

// Start.
//-----------------------------------------------------------------------------

void protocol_transaction_in::start()
{
    protocol_events::start(BIND1(handle_stop, _1));

    // TODO: move memory_pool to a derived class protocol_transaction_in_70002.
    // Prior to this level the mempool message is not available.
    if (refresh_pool_)
    {
        // Refresh transaction pool on connect.
        SEND2(memory_pool(), handle_send, _1, memory_pool::command);

        // Refresh transaction pool on blockchain reorganization.
        blockchain_.subscribe_reorganize(
            BIND4(handle_reorganized, _1, _2, _3, _4));
    }

    SUBSCRIBE2(inventory, handle_receive_inventory, _1, _2);
    SUBSCRIBE2(transaction_message, handle_receive_transaction, _1, _2);
}

// Receive inventory sequence.
//-----------------------------------------------------------------------------

bool protocol_transaction_in::handle_receive_inventory(const code& ec,
    inventory_ptr message)
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

    const auto response = std::make_shared<get_data>();
    message->reduce(response->inventories, inventory::type_id::transaction);

    // TODO: move relay to a derived class protocol_transaction_in_70001.
    // Prior to this level transaction relay is not configurable.
    if (!relay_from_peer_ && !response->inventories.empty())
    {
        log::debug(LOG_NODE)
            << "Unexpected transaction inventory from [" << authority() << "]";
        stop(error::channel_stopped);
        return false;
    }

    // This is returned on a new thread.
    // Remove matching transaction hashes found in the transaction pool.
    pool_.filter(response, BIND2(handle_filter_floaters, _1, response));
    return true;
}

void protocol_transaction_in::handle_filter_floaters(const code& ec,
    get_data_ptr message)
{
    if (stopped() || ec == error::service_stopped ||
        message->inventories.empty())
        return;

    if (ec)
    {
        log::error(LOG_NODE)
            << "Internal failure locating pool transaction pool hashes for ["
            << authority() << "] " << ec.message();
        stop(ec);
        return;
    }

    // BUGBUG: this removes spent transactions which it should not (see BIP30).
    blockchain_.filter_transactions(message,
        BIND2(send_get_data, _1, message));
}

void protocol_transaction_in::send_get_data(const code& ec,
    get_data_ptr message)
{
    if (stopped() || ec == error::service_stopped || 
        message->inventories.empty())
        return;

    if (ec)
    {
        log::error(LOG_NODE)
            << "Internal failure locating confirmed transaction hashes for ["
            << authority() << "] " << ec.message();
        stop(ec);
        return;
    }

    // inventory->get_data[transaction]
    SEND2(*message, handle_send, _1, message->command);
}

// Receive transaction sequence.
//-----------------------------------------------------------------------------

bool protocol_transaction_in::handle_receive_transaction(const code& ec,
    transaction_ptr message)
{
    if (stopped())
        return false;

    if (ec)
    {
        log::debug(LOG_NODE)
            << "Failure getting transaction from [" << authority() << "] "
            << ec.message();
        stop(ec);
        return false;
    }

    // TODO: move relay to a derived class protocol_transaction_in_70001.
    // Prior to this level transaction relay is not configurable.
    if (!relay_from_peer_)
    {
        log::debug(LOG_NODE)
            << "Unexpected transaction relay from [" << authority() << "]";
        stop(error::channel_stopped);
        return false;
    }

    log::debug(LOG_NODE)
        << "Potential transaction from [" << authority() << "].";

    pool_.store(message,
        BIND2(handle_store_confirmed, _1, _2),
        BIND3(handle_store_validated, _1, _2, _3));
    return true;
}

// The transaction has been saved to the memory pool (or not).
// This will be picked up by subscription in transaction_out and will cause
// the transaction to be announced to non-originating relay-accepting peers.
void protocol_transaction_in::handle_store_validated(const code& ec,
    transaction_ptr message, const index_list& unconfirmed)
{
    // Examples:
    // error::service_stopped
    // error::input_not_found
    // error::validate_inputs_failed
    // error::duplicate
    // error::success (transaction is valid and indexed into the mempool)
}

// The transaction has been confirmed in a block.
void protocol_transaction_in::handle_store_confirmed(const code& ec,
    transaction_ptr message)
{
    // Examples:
    // error::service_stopped
    // error::pool_filled
    // error::double_spend
    // error::blockchain_reorganized
    // error::success (tx was found in a block and removed from the mempool)
}

// Subscription.
//-----------------------------------------------------------------------------

// TODO: move memory_pool to a derived class protocol_transaction_in_70002.
// Prior to this level the mempool message is not available.
bool protocol_transaction_in::handle_reorganized(const code& ec, size_t,
    const block_ptr_list&, const block_ptr_list& outgoing)
{
    if (stopped() || ec == error::service_stopped)
        return false;

    if (ec)
    {
        log::error(LOG_NODE)
            << "Internal failure handling reorganization for ["
            << authority() << "] " << ec.message();
        stop(ec);
        return false;
    }

    // If there are no outgoing blocks then the memory pool is intact.
    if (outgoing.empty())
        return true;

    SEND2(memory_pool(), handle_send, _1, memory_pool::command);
    return true;
}

// Stop.
//-----------------------------------------------------------------------------

void protocol_transaction_in::handle_stop(const code&)
{
    log::debug(LOG_NETWORK)
        << "Stopped transaction_in protocol";
}

} // namespace node
} // namespace libbitcoin
