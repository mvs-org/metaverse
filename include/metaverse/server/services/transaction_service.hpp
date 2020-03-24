/**
 * Copyright (c) 2011-2020 libbitcoin developers (see AUTHORS)
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
#ifndef MVS_SERVER_TRANSACTION_SERVICE_HPP
#define MVS_SERVER_TRANSACTION_SERVICE_HPP

#include <memory>
#include <metaverse/protocol.hpp>
#include <metaverse/server/define.hpp>
#include <metaverse/server/settings.hpp>

namespace libbitcoin {
namespace server {

class server_node;

// This class is thread safe.
// Subscribe to transaction acceptances into the transaction memory pool.
class BCS_API transaction_service
  : public bc::protocol::zmq::worker
{
public:
    typedef std::shared_ptr<transaction_service> ptr;

    /// The fixed inprocess worker endpoints.
    static const config::endpoint public_worker;
    static const config::endpoint secure_worker;

    /// Construct a transaction service.
    transaction_service(bc::protocol::zmq::authenticator& authenticator,
        server_node& node, bool secure);

    /// Start the service.
    bool start() override;

    /// Stop the service.
    bool stop() override;

protected:
    typedef bc::protocol::zmq::socket socket;

    virtual bool bind(socket& xpub, socket& xsub);
    virtual bool unbind(socket& xpub, socket& xsub);

    // Implement the service.
    virtual void work() override;

private:
    typedef bc::chain::point::indexes index_list;

    bool handle_transaction(const code& ec, const index_list&,
        bc::message::transaction_message::ptr tx);
    void publish_transaction(const chain::transaction& tx);

    const bool secure_;
    const server::settings& settings_;

    // These are thread safe.
    bc::protocol::zmq::authenticator& authenticator_;
    server_node& node_;
};

} // namespace server
} // namespace libbitcoin

#endif
