/*
* Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS).
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

#include <thread>
#include <functional>
#include <metaverse/mgbubble/MgServer.hpp>
#include <metaverse/bitcoin/utility/log.hpp>
#include <metaverse/bitcoin/version.hpp>

namespace mgbubble {
using namespace std::placeholders;

bool MgServer::start()
{
    if (!nc_)
        return false;
    if (running_)
        return true;

    running_ = true;
    worker_ = std::make_shared<std::thread>([this]() { this->run(); });
    return !!worker_;
}

void MgServer::stop()
{
    if (running_) {
        running_ = false;
        if (!!worker_)
            worker_->join();
    }
}

bool MgServer::broadcast(const std::string& msg)
{
    if (!nc_ || !running_)
        return false;

    mg_broadcast(&mgr_, ev_broadcast, (void*)(msg.data()), msg.size() + 1);
    return true;
}

bool MgServer::broadcast(const char* msg, size_t len)
{
    if (!nc_ || !running_)
        return false;

    mg_broadcast(&mgr_, ev_broadcast, (void*)(msg), len + 1);
    return true;
}

bool MgServer::send(struct mg_connection& nc, const std::string& msg, bool close_required)
{
    if (!nc_ || !running_)
        return false;

    mg_send(&nc, msg.c_str(), msg.size());
    if(close_required)
        nc.flags |= MG_F_SEND_AND_CLOSE;
    return true;
}

bool MgServer::send(struct mg_connection& nc, const char* msg, size_t len, bool close_required)
{
    if (!nc_ || !running_)
        return false;

    mg_send(&nc, msg, len);
    if (close_required)
        nc.flags |= MG_F_SEND_AND_CLOSE;
    return true;
}

bool MgServer::send_frame(struct mg_connection& nc, const std::string& msg, bool binary)
{
    if (!nc_ || !running_)
        return false;

    mg_send_websocket_frame(&nc, (binary ? WEBSOCKET_OP_BINARY : WEBSOCKET_OP_TEXT), (void*)(msg.data()), msg.size());
    return true;
}

bool MgServer::send_frame(struct mg_connection& nc, const char* msg, size_t len, bool binary)
{
    if (!nc_ || !running_)
        return false;

    mg_send_websocket_frame(&nc, (binary ? WEBSOCKET_OP_BINARY : WEBSOCKET_OP_TEXT), (void*)(msg), len);
    return true;
}

void MgServer::run() {
    while (running_)
    {
        mg_mgr_poll(&mgr_, 1000);
    }
}

void MgServer::on_http_req_handler(struct mg_connection& nc, http_message& msg)
{
    mg_http_send_error(&nc, 403, nullptr);
}

// demo
void MgServer::on_ws_handshake_req_handler(struct mg_connection& nc, http_message& msg)
{
    if ((mg_ncasecmp(msg.uri.p, "/ws", 3) != 0) && (mg_ncasecmp(msg.uri.p, "/ws/", 4) != 0)) {
        nc.flags |= MG_F_SEND_AND_CLOSE;
    }
}

void MgServer::on_ws_handshake_done_handler(struct mg_connection& nc)
{
}

// echo
void MgServer::on_ws_frame_handler(struct mg_connection& nc, websocket_message& msg)
{
    struct mg_str d = { (char *)msg.data, msg.size };
    bc::log::info(NAME) << std::string(d.p, d.len);
    mg_send_websocket_frame(&nc, WEBSOCKET_OP_TEXT, msg.data, msg.size);
}

void MgServer::on_ws_ctrlf_handler(struct mg_connection& nc, websocket_message& msg)
{
}

void MgServer::on_timer_handler(struct mg_connection& nc)
{
}

void MgServer::on_close_handler(struct mg_connection& nc)
{
}

void MgServer::on_send_handler(struct mg_connection& nc, int bytes_transfered)
{
}

void MgServer::on_notify_handler(struct mg_connection& nc, struct mg_event& ev)
{
}

void MgServer::on_broadcast(struct mg_connection& nc, const char* ev_data)
{
}

void MgServer::ev_handler_default(struct mg_connection *nc, int ev, void *ev_data)
{
}

void MgServer::ev_broadcast(struct mg_connection *nc, int ev, void *ev_data)
{
    auto* self = dynamic_cast<MgServer*>(static_cast<MgServer*>(nc->mgr->user_data));
    if (!self || self->stopped())
        return;

    self->on_broadcast(*nc, (const char*)ev_data);
}

void MgServer::ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
    auto* self = dynamic_cast<MgServer*>(static_cast<MgServer*>(nc->mgr->user_data));
    if (!self || self->stopped())
        return;
    switch (ev)
    {
    case MG_EV_SEND: {
        self->on_send_handler(*nc, *((int*)ev_data));
        break;
    }
    case MG_EV_HTTP_REQUEST: {
        struct http_message *msg = (struct http_message *)ev_data;
        self->on_http_req_handler(*nc, *msg);
        break;
    }
    case MG_EV_WEBSOCKET_HANDSHAKE_REQUEST: {
        struct http_message *msg = (struct http_message *)ev_data;
        self->on_ws_handshake_req_handler(*nc, *msg);
        break;
    }
    case MG_EV_WEBSOCKET_HANDSHAKE_DONE: {
        self->on_ws_handshake_done_handler(*nc);
        break;
    }
    case MG_EV_WEBSOCKET_FRAME: {
        struct websocket_message *msg = (struct websocket_message *)ev_data;
        self->on_ws_frame_handler(*nc, *msg);
        break;
    }
    case MG_EV_WEBSOCKET_CONTROL_FRAME: {
        struct websocket_message *msg = (struct websocket_message *)ev_data;
        self->on_ws_ctrlf_handler(*nc, *msg);
        break;
    }
    case MG_EV_TIMER: {
        self->on_timer_handler(*nc);
        break;
    }
    case MG_EV_CLOSE: {
        self->on_close_handler(*nc);
        break;
    }
    default:
        self->ev_handler_default(nc, ev, ev_data);
    }
}

void MgServer::ev_notify_handler(struct mg_connection *nc, int ev, void *ev_data)
{
    auto* self = dynamic_cast<MgServer*>(static_cast<MgServer*>(nc->mgr->user_data));
    if (!self || self->stopped())
        return;
    switch (ev)
    {
    case MG_EV_RECV: {
        if (nc->flags & MG_F_USER_2)
        {
            assert(nc->recv_mbuf.len >= sizeof(struct mg_event));
            assert(static_cast<size_t>(*((int*)(ev_data))) >= sizeof(struct mg_event));
            size_t len = 0;
            size_t maxlen = nc->recv_mbuf.len / sizeof(struct mg_event) * sizeof(struct mg_event);
            for (; len < maxlen; len += sizeof(struct mg_event)) {
                struct mg_event* pev = (struct mg_event*)(nc->recv_mbuf.buf + len);
                self->on_notify_handler(*nc, *pev);
            }
            mbuf_remove(&nc->recv_mbuf, len);
        }
        break;
    }
    }
}

}
