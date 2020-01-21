/**
 * Copyright (c) 2019-2020 libbitcoin developers (see AUTHORS)
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
#include <metaverse/server/services/block_service.hpp>

#include <cstdint>
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
using namespace bc::protocol;

static const auto domain = "block";
const config::endpoint block_service::public_worker("inproc://public_block");
const config::endpoint block_service::secure_worker("inproc://secure_block");

block_service::block_service(zmq::authenticator& authenticator,
    server_node& node, bool secure)
  : worker(node.thread_pool()),
    secure_(secure),
    settings_(node.server_settings()),
    authenticator_(authenticator),
    node_(node)
{
}

// There is no unsubscribe so this class shouldn't be restarted.
bool block_service::start()
{
    // Subscribe to blockchain reorganizations.
    node_.subscribe_blockchain(
        std::bind(&block_service::handle_reorganization,
            this, _1, _2, _3, _4));

    return zmq::worker::start();
}

// No unsubscribe so must be kept in scope until subscriber stop complete.
bool block_service::stop()
{
    return zmq::worker::stop();
}

// Implement worker as extended pub-sub.
// The publisher drops messages for lost peers (clients) and high water.
void block_service::work()
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

bool block_service::bind(zmq::socket& xpub, zmq::socket& xsub)
{
    const auto security = secure_ ? "secure" : "public";
    const auto& worker = secure_ ? secure_worker : public_worker;
    const auto& service = secure_ ? settings_.secure_block_endpoint :
        settings_.public_block_endpoint;

    if (!authenticator_.apply(xpub, domain, secure_))
        return false;

    auto ec = xpub.bind(service);

    if (ec)
    {
        log::error(LOG_SERVER)
            << "Failed to bind " << security << " block service to "
            << service << " : " << ec.message();
        return false;
    }

    ec = xsub.bind(worker);

    if (ec)
    {
        log::error(LOG_SERVER)
            << "Failed to bind " << security << " block workers to "
            << worker << " : " << ec.message();
        return false;
    }

    log::info(LOG_SERVER)
        << "Bound " << security << " block service to " << service;
    return true;
}

bool block_service::unbind(zmq::socket& xpub, zmq::socket& xsub)
{
    // Stop both even if one fails.
    const auto service_stop = xpub.stop();
    const auto worker_stop = xsub.stop();
    const auto security = secure_ ? "secure" : "public";

    if (!service_stop)
        log::error(LOG_SERVER)
            << "Failed to unbind " << security << " block service.";

    if (!worker_stop)
        log::error(LOG_SERVER)
            << "Failed to unbind " << security << " block workers.";

    // Don't log stop success.
    return service_stop && worker_stop;
}

// Publish (integral worker).
// ----------------------------------------------------------------------------

bool block_service::handle_reorganization(const code& ec, uint64_t fork_point,
    const block_list& new_blocks, const block_list&)
{
    if (stopped() || ec.value() == error::service_stopped)
        return false;

    if (ec.value() == error::mock)
        return true;

    if (ec)
    {
        log::warning(LOG_SERVER)
            << "Failure handling new block: " << ec.message();

        // Don't let a failure here prevent prevent future notifications.
        return true;
    }

    // Blockchain height is 64 bit but obelisk protocol is 32 bit.
    BITCOIN_ASSERT(fork_point <= max_uint32);
    const auto fork_point32 = static_cast<uint32_t>(fork_point);

    publish_blocks(fork_point32, new_blocks);
    return true;
}

void block_service::publish_blocks(uint32_t fork_point,
    const block_list& blocks)
{
    if (stopped())
        return;

    const auto security = secure_ ? "secure" : "public";
    const auto& endpoint = secure_ ? block_service::secure_worker :
        block_service::public_worker;

    // Subscriptions are off the pub-sub thread so this must connect back.
    // This could be optimized by caching the socket as thread static.
    zmq::socket publisher(authenticator_, zmq::socket::role::publisher);
    const auto ec = publisher.connect(endpoint);

    if (ec.value() == error::service_stopped)
        return;

    if (ec)
    {
        log::warning(LOG_SERVER)
            << "Failed to connect " << security << " block worker: "
            << ec.message();
        return;
    }

    BITCOIN_ASSERT(blocks.size() <= max_uint32);
    BITCOIN_ASSERT(fork_point < max_uint32 - blocks.size());
    auto height = fork_point;

    for (const auto block: blocks)
        publish_block(publisher, height++, block);
}

// [ height:4 ]
// [ header:80 ]
// [ txs... ]
// The payload for block publication is delimited within the zeromq message.
// This is required for compatability and inconsistent with query payloads.
void block_service::publish_block(zmq::socket& publisher, uint32_t height,
    const block_ptr block)
{
    if (stopped())
        return;

    const auto security = secure_ ? "secure" : "public";

    zmq::message broadcast;
    broadcast.enqueue_little_endian(height);
    broadcast.enqueue(block->to_data(false));
    const auto ec = publisher.send(broadcast);

    if (ec.value() == error::service_stopped)
        return;

    if (ec)
    {
        log::warning(LOG_SERVER)
            << "Failed to publish " << security << " bloc ["
            << encode_hash(block->header.hash()) << "] " << ec.message();
        return;
    }

    // This isn't actually a request, should probably update settings.
    log::debug(LOG_SERVER)
        << "Published " << security << " block ["
        << encode_hash(block->header.hash()) << "]";
}

} // namespace server
} // namespace libbitcoin
