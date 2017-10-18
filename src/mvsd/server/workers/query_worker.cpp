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
#include <metaverse/server/workers/query_worker.hpp>

#include <functional>
#include <string>
#include <metaverse/protocol.hpp>
#include <metaverse/server/define.hpp>
#include <metaverse/server/interface/address.hpp>
#include <metaverse/server/interface/blockchain.hpp>
#include <metaverse/server/interface/protocol.hpp>
#include <metaverse/server/interface/transaction_pool.hpp>
#include <metaverse/server/messages/message.hpp>
#include <metaverse/server/server_node.hpp>

namespace libbitcoin {
namespace server {

using namespace std::placeholders;
using namespace bc::protocol;

query_worker::query_worker(zmq::authenticator& authenticator,
    server_node& node, bool secure)
  : worker(node.thread_pool()),
    secure_(secure),
    settings_(node.server_settings()),
    node_(node),
    authenticator_(authenticator)
{
    // The same interface is attached to the secure and public interfaces.
    attach_interface();
}

// Implement worker as a router to the query service.
// v2 libbitcoin-client DEALER does not add delimiter frame.
// The router drops messages for lost peers (query service) and high water.
void query_worker::work()
{
    zmq::socket router(authenticator_, zmq::socket::role::router);

    // Connect socket to the service endpoint.
    if (!started(connect(router)))
        return;

    zmq::poller poller;
    poller.add(router);

    while (!poller.terminated() && !stopped())
    {
        if (poller.wait().contains(router.id()))
            query(router);
    }

    // Disconnect the socket and exit this thread.
    finished(disconnect(router));
}

// Connect/Disconnect.
//-----------------------------------------------------------------------------

bool query_worker::connect(zmq::socket& router)
{
    const auto security = secure_ ? "secure" : "public";
    const auto& endpoint = secure_ ? query_service::secure_query :
        query_service::public_query;

    const auto ec = router.connect(endpoint);

    if (ec)
    {
        log::error(LOG_SERVER)
            << "Failed to connect " << security << " query worker to "
            << endpoint << " : " << ec.message();
        return false;
    }

    log::debug(LOG_SERVER)
        << "Connected " << security << " query worker to " << endpoint;
    return true;
}

bool query_worker::disconnect(zmq::socket& router)
{
    const auto security = secure_ ? "secure" : "public";

    // Don't log stop success.
    if (router.stop())
        return true;

    log::error(LOG_SERVER)
        << "Failed to disconnect " << security << " query worker.";
    return false;
}

// Query Execution.
//-----------------------------------------------------------------------------

// Because the socket is a router we may simply drop invalid queries.
// As a single thread worker this router should not reach high water.
// If we implemented as a replier we would need to always provide a response.
void query_worker::query(zmq::socket& router)
{
    if (stopped())
        return;

    // TODO: rewrite the serial blockchain interface to avoid callbacks.
    // We are using a closure vs. bind to take advantage of move arg syntax.
    const auto sender = [&router](message&& response)
    {
        const auto ec = response.send(router);

        if (ec && ec != (code)error::service_stopped)
            log::warning(LOG_SERVER)
                << "Failed to send query response to "
                << response.route().display() << " " << ec.message();
    };

    message request(secure_);
    const auto ec = request.receive(router);

    if (ec == (code)error::service_stopped)
        return;

    if (ec)
    {
        log::debug(LOG_SERVER)
            << "Failed to receive query from " << request.route().display()
            << " " << ec.message();

        // Because the query did not parse this is likely to be misaddressed.
        sender(message(request, ec));
        return;
    }

    // Locate the request handler for this command.
    const auto handler = command_handlers_.find(request.command());

    if (handler == command_handlers_.end())
    {
        log::debug(LOG_SERVER)
            << "Invalid query command from " << request.route().display();

        sender(message(request, error::not_found));
        return;
    }

    log::info(LOG_SERVER)
        << "Query " << request.command() << " from "
        << request.route().display();

    // The query executor is the delegate bound by the attach method.
    const auto& query_execute = handler->second;

    // Execute the request and forward result to queue.
    // Example: address.renew(node_, request, sender);
    // Example: blockchain.fetch_history(node_, request, sender);
    query_execute(request, sender);
}

// Query Interface.
// ----------------------------------------------------------------------------

// Class and method names must match protocol expectations (do not change).
#define ATTACH(class_name, method_name, node) \
    attach(#class_name "." #method_name, \
        std::bind(&bc::server::class_name::method_name, \
            std::ref(node), _1, _2));

void query_worker::attach(const std::string& command,
    command_handler handler)
{
    command_handlers_[command] = handler;
}

//=============================================================================
// TODO: add to client:
// blockchain.fetch_spend
// blockchain.fetch_block_transaction_hashes
//=============================================================================
// address.fetch_history was present in v1 (obelisk) and v2 (server).
// address.fetch_history was called by client v1 (sx) and v2 (bx).
//-----------------------------------------------------------------------------
// address.renew is deprecated in v3.
// address.subscribe is deprecated in v3.
// address.subscribe2 is new in v3, also call for renew.
// address.unsubscribe2 is new in v3 (there was never an address.unsubscribe).
//-----------------------------------------------------------------------------
///protocol.fetch_stealth is deprecated in v3.
// protocol.fetch_stealth2 is new in v3.
//-----------------------------------------------------------------------------
// blockchain.broadcast_transaction is deprecated in v3 (deferred).
// transaction_pool.broadcast (with radar) is new in v3 (deferred).
//=============================================================================
// Interface class.method names must match protocol (do not change).
void query_worker::attach_interface()
{
    ATTACH(address, renew, node_);
    ATTACH(address, subscribe, node_);
    ATTACH(address, subscribe2, node_);
    ATTACH(address, unsubscribe2, node_);
    ATTACH(address, fetch_history2, node_);
    ATTACH(blockchain, fetch_history, node_);
    ATTACH(blockchain, fetch_block_header, node_);
    ATTACH(blockchain, fetch_block_height, node_);
    ATTACH(blockchain, fetch_block_transaction_hashes, node_);
    ATTACH(blockchain, fetch_last_height, node_);
    ATTACH(blockchain, fetch_transaction, node_);
    ATTACH(blockchain, fetch_transaction_index, node_);
    ATTACH(blockchain, fetch_spend, node_);
    ATTACH(blockchain, fetch_stealth, node_);
    ATTACH(blockchain, fetch_stealth2, node_);
    ATTACH(transaction_pool, fetch_transaction, node_);
    ATTACH(transaction_pool, validate, node_);
    ////ATTACH(transaction_pool, broadcast, node_);
    ATTACH(protocol, broadcast_transaction, node_);
    ATTACH(protocol, total_connections, node_);
}

#undef ATTACH

} // namespace server
} // namespace libbitcoin
