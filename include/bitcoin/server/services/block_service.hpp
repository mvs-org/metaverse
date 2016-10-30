/**
 * Copyright (c) 2011-2016 libbitcoin developers (see AUTHORS)
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
#ifndef LIBBITCOIN_SERVER_BLOCK_SERVICE_HPP
#define LIBBITCOIN_SERVER_BLOCK_SERVICE_HPP

#include <cstdint>
#include <memory>
#include <bitcoin/protocol.hpp>
#include <bitcoin/server/define.hpp>
#include <bitcoin/server/settings.hpp>

namespace libbitcoin {
namespace server {

class server_node;

// This class is thread safe.
// Subscribe to block acceptances into the long chain.
class BCS_API block_service
  : public bc::protocol::zmq::worker
{
public:
    typedef std::shared_ptr<block_service> ptr;

    /// The fixed inprocess worker endpoints.
    static const config::endpoint public_worker;
    static const config::endpoint secure_worker;

    /// Construct a block service.
    block_service(bc::protocol::zmq::authenticator& authenticator,
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
    virtual void work();

private:
    typedef bc::message::block_message::ptr block_ptr;
    typedef bc::message::block_message::ptr_list block_list;

    bool handle_reorganization(const code& ec, uint64_t fork_point,
        const block_list& new_blocks, const block_list&);
    void publish_blocks(uint32_t fork_point, const block_list& blocks);
    void publish_block(socket& publisher, uint32_t height,
        const block_ptr block);

    const bool secure_;
    const server::settings& settings_;

    // These are thread safe.
    bc::protocol::zmq::authenticator& authenticator_;
    server_node& node_;
};

} // namespace server
} // namespace libbitcoin

#endif
