/*
* Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS).
* Copyright (C) 2013, 2016 Swirly Cloud Limited.
*
* This program is free software; you can redistribute it and/or modify it under the terms of the
* GNU General Public License as published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
* even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with this program; if
* not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301, USA.
*/
#ifndef MVSD_WS_PUSH_SERV_HPP
#define MVSD_WS_PUSH_SERV_HPP

#include <atomic>
#include <metaverse/bitcoin.hpp>
#include <metaverse/mgbubble/MgServer.hpp>

namespace libbitcoin {
    namespace server {
        class server_node;
    }
}

namespace mgbubble {
class WsPushServ : public MgServer {
    typedef bc::chain::point::indexes index_list;
    typedef bc::message::block_message::ptr_list block_list;
    typedef MgServer base;

public:
    explicit WsPushServ(libbitcoin::server::server_node& node, const std::string& srv_addr)
        : node_(node), MgServer(srv_addr)
    {}

    bool start() override;

protected:
    bool handle_blockchain_reorganization(const bc::code& ec, uint64_t fork_point, const block_list& new_blocks, const block_list&);
    bool handle_transaction_pool(const bc::code& ec, const index_list&, bc::message::transaction_message::ptr tx);

    void notify_blocks(uint32_t fork_point, const block_list& blocks);
    void notify_block(uint32_t height, const bc::chain::block::ptr block);

    void notify_transaction(uint32_t height, const bc::hash_digest& block_hash, const bc::chain::transaction& tx);

protected:
    void run() override;

    void on_ws_handshake_done_handler(struct mg_connection& nc) override;
    void on_ws_frame_handler(struct mg_connection& nc, websocket_message& msg) override;
    void on_close_handler(struct mg_connection& nc) override;
    void on_broadcast(struct mg_connection& nc, const char* ev_data) override;

private:
    libbitcoin::server::server_node& node_;
};
}

#endif