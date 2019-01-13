/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
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
#ifndef MVS_NODE_SESSION_HEADER_SYNC_HPP
#define MVS_NODE_SESSION_HEADER_SYNC_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include <metaverse/blockchain.hpp>
#include <metaverse/network.hpp>
#include <metaverse/node/define.hpp>
#include <metaverse/node/settings.hpp>
#include <metaverse/node/protocols/protocol_header_sync.hpp>
#include <metaverse/node/protocols/protocol_version_quiet.hpp>
#include <metaverse/node/utility/check_list.hpp>
#include <metaverse/node/utility/header_list.hpp>

namespace libbitcoin {
namespace node {

/// Class to manage initial header download connection, thread safe.
class BCN_API session_header_sync
  : public network::session_batch, track<session_header_sync>
{
public:
    typedef std::shared_ptr<session_header_sync> ptr;

    session_header_sync(network::p2p& network, check_list& hashes,
        blockchain::simple_chain& blockchain,
        const config::checkpoint::list& checkpoints);

    virtual void start(result_handler handler) override;

protected:
    /// Overridden to attach and start specialized handshake.
    void attach_handshake_protocols(network::channel::ptr channel,
        result_handler handle_started) override;

    /// Override to attach and start specialized protocols after handshake.
    virtual void attach_protocols(network::channel::ptr channel,
        header_list::ptr headers, result_handler handler);

private:
    typedef std::vector<header_list::ptr> headers_table;

    bool initialize();

    void handle_started(const code& ec, result_handler handler);

    void new_connection(header_list::ptr row, result_handler handler);

    void start_syncing(const code& ec, const config::authority& host,
        network::connector::ptr connect, result_handler handler);
    void handle_connect(const code& ec, network::channel::ptr channel,
        header_list::ptr row, result_handler handler);

    void handle_complete(const code& ec, header_list::ptr row,
        result_handler handler);

    void handle_channel_start(const code& ec, network::channel::ptr channel,
        header_list::ptr row,
        result_handler handler);

    void handle_channel_stop(const code& ec, header_list::ptr row);

    // Thread safe.
    check_list& hashes_;

    // These do not require guard because they are not used concurrently.
    headers_table headers_;
    uint32_t minimum_rate_;
    blockchain::simple_chain& blockchain_;
    const config::checkpoint::list checkpoints_;
};

} // namespace node
} // namespace libbitcoin

#endif

