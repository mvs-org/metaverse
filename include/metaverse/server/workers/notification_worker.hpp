/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
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
#ifndef MVS_SERVER_NOTIFICATION_WORKER_HPP
#define MVS_SERVER_NOTIFICATION_WORKER_HPP

#include <cstdint>
#include <memory>
#include <metaverse/bitcoin.hpp>
#include <metaverse/server/define.hpp>
#include <metaverse/server/messages/message.hpp>
#include <metaverse/server/messages/route.hpp>
#include <metaverse/server/settings.hpp>
#include <metaverse/server/utility/address_key.hpp>

namespace libbitcoin {
namespace server {

class server_node;

// This class is thread safe.
// Provide address and stealth notifications to the query service.
class BCS_API notification_worker
  : public bc::protocol::zmq::worker
{
public:
    typedef std::shared_ptr<notification_worker> ptr;

    /// Construct an address worker.
    notification_worker(bc::protocol::zmq::authenticator& authenticator,
        server_node& node, bool secure);

    /// Start the worker.
    bool start() override;

    /// Stop the worker.
    bool stop() override;

    /// Subscribe to address and stealth prefix notifications.
    virtual void subscribe_address(const route& reply_to, uint32_t id,
        const binary& prefix_filter, chain::subscribe_type type);

    /// Subscribe to transaction penetration notifications.
    virtual void subscribe_penetration(const route& reply_to, uint32_t id,
        const hash_digest& tx_hash);

protected:
    typedef bc::protocol::zmq::socket socket;

    virtual bool connect(socket& router);
    virtual bool disconnect(socket& router);

    // Implement the service.
    virtual void work() override;

private:
    typedef chain::point::indexes index_list;
    typedef std::shared_ptr<uint8_t> sequence_ptr;
    typedef bc::message::block_message::ptr_list block_list;

    typedef notifier<address_key, const code&,
        const wallet::payment_address&, int32_t, const hash_digest&,
        const chain::transaction&> payment_subscriber;
    typedef notifier<address_key, const code&, uint32_t, uint32_t,
        const hash_digest&, const chain::transaction&> stealth_subscriber;
    typedef notifier<address_key, const code&, const binary&, uint32_t,
        const hash_digest&, const chain::transaction&> address_subscriber;
    typedef notifier<address_key, const code&, uint32_t,
        const hash_digest&, const hash_digest&> penetration_subscriber;

    // Remove expired subscriptions.
    void purge();
    int32_t purge_interval_milliseconds() const;

    bool handle_blockchain_reorganization(const code& ec, uint64_t fork_point,
        const block_list& new_blocks, const block_list&);
    bool handle_transaction_pool(const code& ec, const index_list&,
        bc::message::transaction_message::ptr tx);
    bool handle_inventory(const code& ec,
        const bc::message::inventory::ptr packet);

    void notify_blocks(uint32_t fork_point, const block_list& blocks);
    void notify_block(socket& peer, uint32_t height,
        const chain::block::ptr block);
    void notify_transaction(uint32_t height, const hash_digest& block_hash,
        const chain::transaction& tx);

    // v2/v3 (deprecated)
    void notify_payment(const wallet::payment_address& address,
        uint32_t height, const hash_digest& block_hash,
        const chain::transaction& tx);
    void notify_stealth(uint32_t prefix, uint32_t height,
        const hash_digest& block_hash, const chain::transaction& tx);

    // v3
    void notify_address(const binary& field, uint32_t height,
        const hash_digest& block_hash, const chain::transaction& tx);
    void notify_penetration(uint32_t height, const hash_digest& block_hash,
        const hash_digest& tx_hash);

    // Send a notification to the subscriber.
    void send(const route& reply_to, const std::string& command,
        uint32_t id, const data_chunk& payload);
    void send_payment(const route& reply_to, uint32_t id,
        const wallet::payment_address& address, uint32_t height,
        const hash_digest& block_hash, const chain::transaction& tx);
    void send_stealth(const route& reply_to, uint32_t id, uint32_t prefix,
        uint32_t height, const hash_digest& block_hash,
        const chain::transaction& tx);
    void send_address(const route& reply_to, uint32_t id, uint8_t sequence,
        uint32_t height, const hash_digest& block_hash,
        const chain::transaction& tx);

    bool handle_payment(const code& ec, const wallet::payment_address& address,
        uint32_t height, const hash_digest& block_hash,
        const chain::transaction& tx, const route& reply_to, uint32_t id,
        const binary& prefix_filter);
    bool handle_stealth(const code& ec, uint32_t prefix, uint32_t height,
        const hash_digest& block_hash, const chain::transaction& tx,
        const route& reply_to, uint32_t id, const binary& prefix_filter);
    bool handle_address(const code& ec, const binary& field, uint32_t height,
        const hash_digest& block_hash, const chain::transaction& tx,
        const route& reply_to, uint32_t id, const binary& prefix_filter,
        sequence_ptr sequence);

    const bool secure_;
    const server::settings& settings_;

    // These are thread safe.
    server_node& node_;
    bc::protocol::zmq::authenticator& authenticator_;
    address_subscriber::ptr address_subscriber_;
    payment_subscriber::ptr payment_subscriber_;
    stealth_subscriber::ptr stealth_subscriber_;
    penetration_subscriber::ptr penetration_subscriber_;
};

} // namespace server
} // namespace libbitcoin

#endif
