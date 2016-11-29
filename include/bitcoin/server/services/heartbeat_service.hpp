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
#ifndef MVS_SERVER_HEARTBEAT_SERVICE_HPP
#define MVS_SERVER_HEARTBEAT_SERVICE_HPP

#include <cstdint>
#include <memory>
#include <bitcoin/protocol.hpp>
#include <bitcoin/server/define.hpp>
#include <bitcoin/server/settings.hpp>

namespace libbitcoin {
namespace server {

class server_node;

// This class is thread safe.
// Subscribe to a pulse from a dedicated service endpoint.
class BCS_API heartbeat_service
  : public bc::protocol::zmq::worker
{
public:
    typedef std::shared_ptr<heartbeat_service> ptr;

    /// Construct a heartbeat endpoint.
    heartbeat_service(bc::protocol::zmq::authenticator& authenticator,
        server_node& node, bool secure);

protected:
    typedef bc::protocol::zmq::socket socket;

    virtual bool bind(socket& publisher);
    virtual bool unbind(socket& publisher);

    // Implement the service.
    virtual void work();

    // Publish the heartbeat (integrated worker).
    void publish(uint32_t count, socket& socket);

private:
    const server::settings& settings_;
    const int32_t period_;
    const bool secure_;

    // This is thread safe.
    bc::protocol::zmq::authenticator& authenticator_;
};

} // namespace server
} // namespace libbitcoin

#endif
