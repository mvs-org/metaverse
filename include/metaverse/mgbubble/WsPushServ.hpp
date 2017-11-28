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

#include <mongoose/mongoose.h>

namespace libbitcoin {
    namespace server {
        class server_node;
    }
}

namespace mgbubble {
class WsPushServ {
    typedef bc::chain::point::indexes index_list;
public:
    WsPushServ(libbitcoin::server::server_node& node) : node_(node), nc_(nullptr), running_(false)
    {
        mg_mgr_init(&mgr_, this);
    }

    virtual ~WsPushServ()
    {
        mg_mgr_free(&mgr_);
    }

    bool start();

private:
    void run();

    bool handle_transaction(const bc::code& ec, const index_list&, bc::message::transaction_message::ptr tx);

    void publish_transaction(const bc::chain::transaction& tx);

    void on_ws_handshake_req_handler(struct mg_connection& nc, http_message& msg);
    void on_ws_handshake_done_handler(struct mg_connection& nc);
    void on_ws_frame_handler(struct mg_connection& nc, websocket_message& msg);
    void on_ws_ctrlf_handler(struct mg_connection& nc, websocket_message& msg);
    void on_timer_handler(struct mg_connection& nc);
    void on_close_handler(struct mg_connection& nc);
    void on_broadcast(struct mg_connection& nc, const struct mg_str& msg);

private:
    static void ev_broadcast(struct mg_connection *nc, int ev, void *ev_data);
    static void ev_handler(struct mg_connection *nc, int ev, void *ev_data);

private:
    struct mg_mgr mgr_;
    struct mg_connection *nc_;
    struct mg_serve_http_opts s_http_server_opts_;

    std::atomic<bool> running_;
    libbitcoin::server::server_node& node_;
};
}

#endif