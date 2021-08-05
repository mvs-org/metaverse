/*
* Copyright (c) 2016-2021 metaverse core developers (see MVS-AUTHORS).
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

#include <metaverse/bitcoin/define.hpp>
#include <atomic>
#include <string>
#include <memory>
#include <mongoose/mongoose.h>

namespace mgbubble {

struct mg_event {
    //uint64_t id;
    void* data;
};

class MgServer {
public:
    static constexpr auto NAME = "MONGO";

public:
    explicit MgServer(const std::string& svr_addr) : svr_addr_(svr_addr), running_(false)
    {
        memset(&s_http_server_opts_, 0x00, sizeof(s_http_server_opts_));

        mg_mgr_init(&mgr_, this);
        nc_ = mg_bind(&mgr_, svr_addr_.c_str(), ev_handler);
        if (nc_ != nullptr)
        {
            nc_->flags |= MG_F_USER_1; // mark as listen socket
            mg_set_protocol_http_websocket(nc_);
        }

        notify_sock_[0] = notify_sock_[1] = INVALID_SOCKET;
        nc_notify_ = nullptr;
    }

    virtual ~MgServer()
    {
        mg_mgr_free(&mgr_);
    }

    virtual bool start();
    virtual void stop();
    bool stopped() { return running_ == false; }
    bool is_websocket(const struct mg_connection& nc) const { return !!(nc.flags & MG_F_IS_WEBSOCKET); }
    bool is_listen_socket(const struct mg_connection& nc) const { return !!(nc.flags & MG_F_USER_1); }
    bool is_on_sending(struct mg_connection& nc) { return (nc.send_mbuf.len != 0); }

    // DO NOT CALL IN WsServer Worker Thread
    // on_broadcast called after broadcasted
    // max buffer is 8k (mongoose default)
    bool broadcast(const std::string& msg);
    bool broadcast(const char* msg, size_t len);

    void set_document_root(const char* root) { s_http_server_opts_.document_root = root; }

    bool attach_notify(mg_event_handler_t callback = nullptr) {
        if ((notify_sock_[0] == INVALID_SOCKET) && (mg_socketpair(notify_sock_, SOCK_STREAM) == 1))
        {
            nc_notify_ = mg_add_sock(&mgr_, notify_sock_[1], (callback != nullptr) ? callback : ev_handler);
            if (nc_notify_ != nullptr)
            {
                nc_notify_->flags |= MG_F_USER_2; // mark as notify socket
                nc_notify_->handler = ev_notify_handler;
            }
        }
        return nc_notify_ != nullptr;
    }
    bool is_notify_socket(struct mg_connection& nc) const { return !!(nc.flags & MG_F_USER_2); }

    bool notify(uint64_t id, void* data) {
        (void)(id);
        struct mg_event ev{data};
        return notify(ev);
    }
    bool notify(struct mg_event ev)
    {
        if (notify_sock_[0] == INVALID_SOCKET)
            return false;
        int n = ::MG_SEND_FUNC(notify_sock_[0], (const char*)&ev, sizeof(ev), 0);
        return n > 0;
    }

protected:
    // ONLY CALLED IN WsServer Worker Thread
    bool send(struct mg_connection& nc, const std::string& msg, bool close_required = false);
    bool send(struct mg_connection& nc, const char* msg, size_t len, bool close_required = false);
    bool send_frame(struct mg_connection& nc, const std::string& msg, bool binary = false);
    bool send_frame(struct mg_connection& nc, const char* msg, size_t len, bool binary = false);

    void serve_http_static(struct mg_connection& nc, struct http_message& hm)
    {
        mg_serve_http(&nc, &hm, s_http_server_opts_);
        nc.flags |= MG_F_SEND_AND_CLOSE;
    }

protected:
    struct mg_mgr& mg_mgr() { return mgr_; }
    struct mg_connection& mg_listen() { return *nc_; }

protected:
    virtual void run();

    virtual void on_http_req_handler(struct mg_connection& nc, http_message& msg);
    virtual void on_ws_handshake_req_handler(struct mg_connection& nc, http_message& msg);
    virtual void on_ws_handshake_done_handler(struct mg_connection& nc);
    virtual void on_ws_frame_handler(struct mg_connection& nc, websocket_message& msg);
    virtual void on_ws_ctrlf_handler(struct mg_connection& nc, websocket_message& msg);
    virtual void on_timer_handler(struct mg_connection& nc);
    virtual void on_close_handler(struct mg_connection& nc);
    virtual void on_send_handler(struct mg_connection& nc, int bytes_transfered);
    virtual void on_notify_handler(struct mg_connection& nc, struct mg_event& ev);

    // called for each connection after broadcast
    virtual void on_broadcast(struct mg_connection& nc, const char* ev_data);

    virtual void ev_handler_default(struct mg_connection *nc, int ev, void *ev_data);

protected:
    static void ev_broadcast(struct mg_connection *nc, int ev, void *ev_data);
    static void ev_handler(struct mg_connection *nc, int ev, void *ev_data);
    static void ev_notify_handler(struct mg_connection *nc, int ev, void *ev_data);

private:
    struct mg_mgr mgr_;
    struct mg_connection *nc_;
    struct mg_serve_http_opts s_http_server_opts_;

    sock_t notify_sock_[2]; // 0 is used out of thread, 1 is used in mongoose event loop
    struct mg_connection *nc_notify_;

    std::string svr_addr_;
    std::atomic<bool> running_;
    std::shared_ptr<std::thread> worker_;
};
}

#endif
