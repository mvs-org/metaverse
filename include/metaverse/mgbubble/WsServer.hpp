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
#ifndef MVSD_WS_SERVER_HPP
#define MVSD_WS_SERVER_HPP

#include <atomic>
#include <string>
#include <mongoose/mongoose.h>

namespace mgbubble {
class WsServer {
public:
    static constexpr auto NAME = "WS";

public:
    explicit WsServer(const std::string& svr_addr) : svr_addr_(svr_addr), running_(false)
    {
        mg_mgr_init(&mgr_, this);
        nc_ = mg_bind(&mgr_, svr_addr_.c_str(), ev_handler);
        if (nc_ != nullptr)
        {
            mg_set_protocol_http_websocket(nc_);
        }
    }

    virtual ~WsServer()
    {
        mg_mgr_free(&mgr_);
    }

    virtual bool start();

protected:
    virtual void run();

    virtual void on_ws_handshake_req_handler(struct mg_connection& nc, http_message& msg);
    virtual void on_ws_handshake_done_handler(struct mg_connection& nc);
    virtual void on_ws_frame_handler(struct mg_connection& nc, websocket_message& msg);
    virtual void on_ws_ctrlf_handler(struct mg_connection& nc, websocket_message& msg);
    virtual void on_timer_handler(struct mg_connection& nc);
    virtual void on_close_handler(struct mg_connection& nc);
    virtual void on_broadcast(struct mg_connection& nc, int ev, void *ev_data);

    virtual void ev_handler_default(struct mg_connection *nc, int ev, void *ev_data);

protected:
    static void ev_broadcast(struct mg_connection *nc, int ev, void *ev_data);
    static void ev_handler(struct mg_connection *nc, int ev, void *ev_data);

protected:
    struct mg_mgr mgr_;
    struct mg_connection *nc_;
    struct mg_serve_http_opts s_http_server_opts_;

    std::string svr_addr_;
    std::atomic<bool> running_;
};
}

#endif