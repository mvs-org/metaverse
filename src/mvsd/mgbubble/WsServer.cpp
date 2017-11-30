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

#include <thread>
#include <functional>
#include <metaverse/mgbubble/WsServer.hpp>
#include <metaverse/bitcoin/utility/log.hpp>

namespace mgbubble {
using namespace std::placeholders;

bool WsServer::start()
{
    if (!nc_)
        return false;
    if (running_)
        return true;

    running_ = true;
    worker_ = std::make_shared<std::thread>([this]() { this->run(); });
    return !!worker_;
}

void WsServer::stop()
{
    if (running_) {
        running_ = false;
        if (!!worker_)
            worker_->join();
    }
}

bool WsServer::broadcast(const std::string& msg)
{
    if (!nc_ || !running_)
        return false;

    mg_broadcast(&mgr_, ev_broadcast, (void*)(msg.data()), msg.size() + 1);
    return true;
}

bool WsServer::broadcast(const char* msg, size_t len)
{
    if (!nc_ || !running_)
        return false;

    mg_broadcast(&mgr_, ev_broadcast, (void*)(msg), len + 1);
    return true;
}

bool WsServer::send(struct mg_connection& nc, const std::string& msg, bool binary)
{
    if (!nc_ || !running_)
        return false;

    mg_send_websocket_frame(&nc, (binary ? WEBSOCKET_OP_BINARY : WEBSOCKET_OP_TEXT), (void*)(msg.data()), msg.size());
    return true;
}

bool WsServer::send(struct mg_connection& nc, const char* msg, size_t len, bool binary)
{
    if (!nc_ || !running_)
        return false;

    mg_send_websocket_frame(&nc, (binary ? WEBSOCKET_OP_BINARY : WEBSOCKET_OP_TEXT), (void*)(msg), len);
    return true;
}

void WsServer::run() {
    bc::log::info(NAME) << "WsServer Started.";
    while (running_)
    {
        mg_mgr_poll(&mgr_, 1000);
    }
    bc::log::info(NAME) << "WsServer Stopped.";
}

// demo
void WsServer::on_ws_handshake_req_handler(struct mg_connection& nc, http_message& msg)
{
    if (memcmp(msg.uri.p, "/ws", 3) != 0 || (msg.uri.len > 3 && msg.uri.p[3] != '/')) {
        nc.flags |= MG_F_SEND_AND_CLOSE;
    }
}

void WsServer::on_ws_handshake_done_handler(struct mg_connection& nc)
{
}

// echo
void WsServer::on_ws_frame_handler(struct mg_connection& nc, websocket_message& msg)
{
    struct mg_str d = { (char *)msg.data, msg.size };
    bc::log::info(NAME) << std::string(d.p, d.len);
    mg_send_websocket_frame(&nc, WEBSOCKET_OP_TEXT, msg.data, msg.size);
}

void WsServer::on_ws_ctrlf_handler(struct mg_connection& nc, websocket_message& msg)
{
}

void WsServer::on_timer_handler(struct mg_connection& nc)
{
}

void WsServer::on_close_handler(struct mg_connection& nc)
{
}

void WsServer::on_broadcast(struct mg_connection& nc, const char* ev_data)
{
}

void WsServer::ev_handler_default(struct mg_connection *nc, int ev, void *ev_data)
{

}

void WsServer::ev_broadcast(struct mg_connection *nc, int ev, void *ev_data)
{
    auto* self = dynamic_cast<WsServer*>(static_cast<WsServer*>(nc->mgr->user_data));
    if (!self)
        return;

    self->on_broadcast(*nc, (const char*)ev_data);
}

void WsServer::ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
    auto* self = dynamic_cast<WsServer*>(static_cast<WsServer*>(nc->mgr->user_data));
    if (!self)
        return;
    switch (ev)
    {
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

}