/**
 * Copyright (c) 2019-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2019-2020 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse.
 *
 * metaverse is free software: you can redistribute it and/or modify
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
#ifndef MVS_NODE_PROTOCOL_HEADER_SYNC_HPP
#define MVS_NODE_PROTOCOL_HEADER_SYNC_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <metaverse/network.hpp>
#include <metaverse/node/configuration.hpp>
#include <metaverse/node/define.hpp>
#include <metaverse/node/utility/header_queue.hpp>

namespace libbitcoin {
namespace node {

/// Headers sync protocol, thread safe.
class BCN_API protocol_header_sync
  : public network::protocol_timer, public track<protocol_header_sync>
{
public:
    typedef std::shared_ptr<protocol_header_sync> ptr;

    /// Construct a header sync protocol instance.
    protocol_header_sync(network::p2p& network, network::channel::ptr channel,
        header_queue& hashes, uint32_t minimum_rate,
        const config::checkpoint& last);

    /// Start the protocol.
    virtual void start(event_handler handler);

private:
    typedef message::headers::ptr headers_ptr;

    static size_t final_height(header_queue& headers,
        const config::checkpoint::list& checkpoints);

    size_t sync_rate() const;
    size_t next_height() const;

    void send_get_headers(event_handler complete);
    void handle_send(const code& ec, event_handler complete);
    void handle_event(const code& ec, event_handler complete);
    void headers_complete(const code& ec, event_handler handler);
    bool handle_receive(const code& ec, headers_ptr message,
        event_handler complete);

    // Thread safe and guarded by sequential header sync.
    header_queue& hashes_;

    // This is guarded by protocol_timer/deadline contract (exactly one call).
    size_t current_second_;

    const uint32_t minimum_rate_;
    const size_t start_size_;
    const config::checkpoint last_;
};

} // namespace node
} // namespace libbitcoin

#endif
