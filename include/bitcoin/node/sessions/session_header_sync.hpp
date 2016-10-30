/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
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
#ifndef LIBBITCOIN_NODE_SESSION_HEADER_SYNC_HPP
#define LIBBITCOIN_NODE_SESSION_HEADER_SYNC_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <bitcoin/blockchain.hpp>
#include <bitcoin/network.hpp>
#include <bitcoin/node/define.hpp>
#include <bitcoin/node/settings.hpp>
#include <bitcoin/node/utility/header_queue.hpp>

namespace libbitcoin {
namespace node {
    
/// Class to manage initial header download connection, thread safe.
class BCN_API session_header_sync
  : public network::session_batch, track<session_header_sync>
{
public:
    typedef std::shared_ptr<session_header_sync> ptr;

    session_header_sync(network::p2p& network, header_queue& hashes,
        blockchain::simple_chain& blockchain,
        const config::checkpoint::list& checkpoints);

    virtual void start(result_handler handler);

protected:
    /// Overridden to attach and start specialized handshake.
    void attach_handshake_protocols(network::channel::ptr channel,
        result_handler handle_started) override;

    /// Override to attach and start specialized protocols after handshake.
    virtual void attach_protocols(network::channel::ptr channel,
        network::connector::ptr connect, result_handler handler);

private:
    bool initialize(result_handler handler);
    void handle_started(const code& ec, result_handler handler);
    void new_connection(network::connector::ptr connect,
        result_handler handler);
    void start_syncing(const code& ec, const config::authority& host,
        network::connector::ptr connect, result_handler handler);
    void handle_connect(const code& ec, network::channel::ptr channel,
        network::connector::ptr connect, result_handler handler);
    void handle_complete(const code& ec, network::connector::ptr connect,
        result_handler handler);
    void handle_channel_start(const code& ec, network::connector::ptr connect,
        network::channel::ptr channel, result_handler handler);
    void handle_channel_stop(const code& ec);
    code get_range(config::checkpoint& out_seed, config::checkpoint& out_stop);

    // Thread safe.
    header_queue& hashes_;

    // These do not require guard because they are not used concurrently.
    uint32_t minimum_rate_;
    config::checkpoint last_;
    blockchain::simple_chain& blockchain_;
    const config::checkpoint::list checkpoints_;
};

} // namespace node
} // namespace libbitcoin

#endif

