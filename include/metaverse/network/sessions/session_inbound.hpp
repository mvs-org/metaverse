/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS)
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
#ifndef MVS_NETWORK_SESSION_INBOUND_HPP
#define MVS_NETWORK_SESSION_INBOUND_HPP

#include <cstddef>
#include <memory>
#include <vector>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/acceptor.hpp>
#include <metaverse/network/channel.hpp>
#include <metaverse/network/define.hpp>
#include <metaverse/network/sessions/session.hpp>
#include <metaverse/network/settings.hpp>

namespace libbitcoin {
namespace network {

class p2p;

/// Inbound connections session, thread safe.
class BCT_API session_inbound
  : public session, track<session_inbound>
{
public:
    typedef std::shared_ptr<session_inbound> ptr;

    /// Construct an instance.
    session_inbound(p2p& network);

    /// Start the session.
    void start(result_handler handler) override;

protected:
    /// Override to attach specialized protocols upon channel start.
    virtual void attach_protocols(channel::ptr channel);

private:
    void start_accept(const code& ec, acceptor::ptr accept);
    void handle_started(const code& ec, result_handler handler);
    void handle_is_loopback(bool loopback, channel::ptr channel);
    void handle_connection_count(size_t connections, channel::ptr channel);
    void handle_accept(const code& ec, channel::ptr channel,
        acceptor::ptr accept);

    void handle_channel_start(const code& ec, channel::ptr channel);
    void handle_channel_stop(const code& ec);
    p2p& network_;
};

} // namespace network
} // namespace libbitcoin

#endif
