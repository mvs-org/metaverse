/**
 * Copyright (c) 2019-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2019-2020 metaverse core developers (see MVS-AUTHORS)
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
#include <metaverse/server/services/transaction_service.hpp>

#include <functional>
#include <memory>
#include <metaverse/protocol.hpp>
#include <metaverse/server/configuration.hpp>
#include <metaverse/server/server_node.hpp>
#include <metaverse/server/settings.hpp>

namespace libbitcoin {
namespace server {

using namespace std::placeholders;
using namespace bc::chain;
using namespace bc::message;
using namespace bc::protocol;

static const auto domain = "transaction";
const config::endpoint transaction_service::public_worker("inproc://public_tx");
const config::endpoint transaction_service::secure_worker("inproc://secure_tx");

transaction_service::transaction_service(zmq::authenticator& authenticator,
    server_node& node, bool secure)
  : worker(node.thread_pool()),
    secure_(secure),
    settings_(node.server_settings()),
    authenticator_(authenticator),
    node_(node)
{
}

// There is no unsubscribe so this class shouldn't be restarted.
bool transaction_service::start()
{
    // Subscribe to transaction pool acceptances.
    node_.subscribe_transaction_pool(
        std::bind(&transaction_service::handle_transaction,
            this, _1, _2, _3));

    return zmq::worker::start();
}


// No unsubscribe so must be kept in scope until subscriber stop complete.
bool transaction_service::stop()
{
    return zmq::worker::stop();
}

// Implement worker as extended pub-sub.
// The publisher drops messages for lost peers (clients) and high water.
void transaction_service::work()
{
    zmq::socket xpub(authenticator_, zmq::socket::role::extended_publisher);
    zmq::socket xsub(authenticator_, zmq::socket::role::extended_subscriber);

    // Bind sockets to the service and worker endpoints.
    if (!started(bind(xpub, xsub)))
        return;

    // TODO: tap in to failure conditions, such as high water.
    // Relay messages between subscriber and publisher (blocks on context).
    relay(xpub, xsub);

    // Unbind the sockets and exit this thread.
    finished(unbind(xpub, xsub));
}

// Bind/Unbind.
//-----------------------------------------------------------------------------

bool transaction_service::bind(zmq::socket& xpub, zmq::socket& xsub)
{
    const auto security = secure_ ? "secure" : "public";
    const auto& worker = secure_ ? secure_worker : public_worker;
    const auto& service = secure_ ? settings_.secure_transaction_endpoint :
        settings_.public_transaction_endpoint;

    if (!authenticator_.apply(xpub, domain, secure_))
        return false;

    auto ec = xpub.bind(service);

    if (ec)
    {
        log::error(LOG_SERVER)
            << "Failed to bind " << security << " transaction service to "
            << service << " : " << ec.message();
        return false;
    }

    ec = xsub.bind(worker);

    if (ec)
    {
        log::error(LOG_SERVER)
            << "Failed to bind " << security << " transaction workers to "
            << worker << " : " << ec.message();
        return false;
    }

    log::info(LOG_SERVER)
        << "Bound " << security << " transaction service to " << service;
    return true;
}

bool transaction_service::unbind(zmq::socket& xpub, zmq::socket& xsub)
{
    // Stop both even if one fails.
    const auto service_stop = xpub.stop();
    const auto worker_stop = xsub.stop();
    const auto security = secure_ ? "secure" : "public";

    if (!service_stop)
        log::error(LOG_SERVER)
            << "Failed to unbind " << security << " transaction service.";

    if (!worker_stop)
        log::error(LOG_SERVER)
            << "Failed to unbind " << security << " transaction workers.";

    // Don't log stop success.
    return service_stop && worker_stop;
}

// Publish (integral worker).
// ----------------------------------------------------------------------------

bool transaction_service::handle_transaction(const code& ec, const index_list&,
    transaction_message::ptr tx)
{
    if (stopped() || ec.value() == error::service_stopped)
        return false;

    if (ec.value() == error::mock)
    {
        return true;
    }

    if (ec)
    {
        log::warning(LOG_SERVER)
            << "Failure handling new transaction: " << ec.message();

        // Don't let a failure here prevent prevent future notifications.
        return true;
    }

    publish_transaction(*tx);
    return true;
}

// [ tx... ]
void transaction_service::publish_transaction(const transaction& tx)
{
    if (stopped())
        return;

    const auto security = secure_ ? "secure" : "public";
    const auto& endpoint = secure_ ? transaction_service::secure_worker :
        transaction_service::public_worker;

    // Subscriptions are off the pub-sub thread so this must connect back.
    // This could be optimized by caching the socket as thread static.
    zmq::socket publisher(authenticator_, zmq::socket::role::publisher);
    auto ec = publisher.connect(endpoint);

    if (ec.value() == error::service_stopped)
        return;

    if (ec)
    {
        log::warning(LOG_SERVER)
            << "Failed to connect " << security << " transaction worker: "
            << ec.message();
        return;
    }

    if (stopped())
        return;

    zmq::message broadcast;
    bc::message::transaction_message tx_msg(tx);
    broadcast.enqueue(tx_msg.to_data(bc::message::version::level::maximum));
    ec = publisher.send(broadcast);

    if (ec.value() == error::service_stopped)
        return;

    if (ec)
    {
        log::warning(LOG_SERVER)
            << "Failed to publish " << security << " transaction ["
            << encode_hash(tx_msg.hash()) << "] " << ec.message();
        return;
    }

    // This isn't actually a request, should probably update settings.
    log::debug(LOG_SERVER)
        << "Published " << security << " transaction ["
        << encode_hash(tx_msg.hash()) << "]";
}

} // namespace server
} // namespace libbitcoin
