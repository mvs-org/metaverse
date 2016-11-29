/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin-server.
 *
 * libbitcoin-server is free software: you can redistribute it and/or
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
#ifndef MVS_SERVER_QUERY_SERVICE_HPP
#define MVS_SERVER_QUERY_SERVICE_HPP

#include <memory>
#include <bitcoin/protocol.hpp>
#include <bitcoin/server/define.hpp>
#include <bitcoin/server/settings.hpp>

namespace libbitcoin {
namespace server {

class server_node;

// This class is thread safe.
// Submit queries and address subscriptions and receive address notifications.
class BCS_API query_service
  : public bc::protocol::zmq::worker
{
public:
    typedef std::shared_ptr<query_service> ptr;

    /// The fixed inprocess query and notify worker endpoints.
    static const config::endpoint public_query;
    static const config::endpoint secure_query;
    static const config::endpoint public_notify;
    static const config::endpoint secure_notify;

    /// Construct a query service.
    query_service(bc::protocol::zmq::authenticator& authenticator,
        server_node& node, bool secure);

protected:
    typedef bc::protocol::zmq::socket socket;

    virtual bool bind(socket& router, socket& query_dealer,
        socket& notify_dealer);
    virtual bool unbind(socket& router, socket& query_dealer,
        socket& notify_dealer);

    // Implement the service.
    virtual void work();

private:
    const bool secure_;
    const server::settings& settings_;

    // This is thread safe.
    bc::protocol::zmq::authenticator& authenticator_;
};

} // namespace server
} // namespace libbitcoin

#endif
