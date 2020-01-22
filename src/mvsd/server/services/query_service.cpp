/**
 * Copyright (c) 2011-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2020 metaverse core developers (see MVS-AUTHORS)
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
#include <metaverse/server/services/query_service.hpp>

#include <metaverse/protocol.hpp>
#include <metaverse/server/server_node.hpp>
#include <metaverse/server/settings.hpp>

namespace libbitcoin {
namespace server {

using namespace bc::config;
using namespace bc::protocol;

static const auto domain = "query";
const config::endpoint query_service::public_query("inproc://public_query");
const config::endpoint query_service::secure_query("inproc://secure_query");
const config::endpoint query_service::public_notify("inproc://public_notify");
const config::endpoint query_service::secure_notify("inproc://secure_notify");

query_service::query_service(zmq::authenticator& authenticator,
    server_node& node, bool secure)
  : worker(node.thread_pool()),
    secure_(secure),
    settings_(node.server_settings()),
    authenticator_(authenticator)
{
}

// Implement worker as a broker.
// The dealer blocks until there are available workers.
// The router drops messages for lost peers (clients) and high water.
void query_service::work()
{
    zmq::socket router(authenticator_, zmq::socket::role::router);
    zmq::socket query_dealer(authenticator_, zmq::socket::role::dealer);
    zmq::socket notify_dealer(authenticator_, zmq::socket::role::dealer);

    // Bind sockets to the service and worker endpoints.
    if (!started(bind(router, query_dealer, notify_dealer)))
        return;

    zmq::poller poller;
    poller.add(router);
    poller.add(query_dealer);
    poller.add(notify_dealer);

    while (!poller.terminated() && !stopped())
    {
        const auto signaled = poller.wait();

        if (signaled.contains(router.id()) &&
            !forward(router, query_dealer))
        {
            log::warning(LOG_SERVER)
                << "Failed to forward from router to query_dealer.";
        }

        if (signaled.contains(query_dealer.id()) &&
            !forward(query_dealer, router))
        {
            log::warning(LOG_SERVER)
                << "Failed to forward from query_dealer to router.";
        }

        if (signaled.contains(notify_dealer.id()) &&
            !forward(notify_dealer, router))
        {
            log::warning(LOG_SERVER)
                << "Failed to forward from notify_dealer to router.";
        }
    }

    // Unbind the sockets and exit this thread.
    finished(unbind(router, query_dealer, notify_dealer));
}

// Bind/Unbind.
//-----------------------------------------------------------------------------

bool query_service::bind(zmq::socket& router, zmq::socket& query_dealer,
    zmq::socket& notify_dealer)
{
    const auto security = secure_ ? "secure" : "public";
    const auto& query_worker = secure_ ? secure_query : public_query;
    const auto& notify_worker = secure_ ? secure_notify : public_notify;
    const auto& query_service = secure_ ? settings_.secure_query_endpoint :
        settings_.public_query_endpoint;

    if (!authenticator_.apply(router, domain, secure_))
        return false;

    auto ec = router.bind(query_service);

    if (ec)
    {
        log::error(LOG_SERVER)
            << "Failed to bind " << security << " query service to "
            << query_service << " : " << ec.message();
        return false;
    }

    ec = query_dealer.bind(query_worker);

    if (ec)
    {
        log::error(LOG_SERVER)
            << "Failed to bind " << security << " query workers to "
            << query_worker << " : " << ec.message();
        return false;
    }

    ec = notify_dealer.bind(notify_worker);

    if (ec)
    {
        log::error(LOG_SERVER)
            << "Failed to bind " << security << " notify workers to "
            << notify_worker << " : " << ec.message();
        return false;
    }

    log::info(LOG_SERVER)
        << "Bound " << security << " query service to " << query_service;
    return true;
}

bool query_service::unbind(zmq::socket& router, zmq::socket& query_dealer,
    zmq::socket& notify_dealer)
{
    // Stop all even if one fails.
    const auto service_stop = router.stop();
    const auto query_stop = query_dealer.stop();
    const auto notify_stop = notify_dealer.stop();
    const auto security = secure_ ? "secure" : "public";

    if (!service_stop)
        log::error(LOG_SERVER)
            << "Failed to unbind " << security << " query service.";

    if (!query_stop)
        log::error(LOG_SERVER)
            << "Failed to unbind " << security << " query workers.";

    if (!notify_stop)
        log::error(LOG_SERVER)
            << "Failed to unbind " << security << " notify workers.";

    // Don't log stop success.
    return service_stop && query_stop && notify_stop;
}

} // namespace server
} // namespace libbitcoin
