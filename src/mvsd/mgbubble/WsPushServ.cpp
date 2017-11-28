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
#include <metaverse/mgbubble/WsPushServ.hpp>
#include <metaverse/server/server_node.hpp>

#define NAME "PUSH"
namespace mgbubble {
using namespace std::placeholders;

bool WsPushServ::start()
{
    const char* svr_addr = node_.server_settings().ws_stream_listen.c_str();
    nc_ = mg_bind(&mgr_, svr_addr, ev_handler);
    if (!nc_)
        return false;

    nc_->flags |= MG_F_USER_1;
    mg_set_protocol_http_websocket(nc_);
    mg_set_timer(nc_, mg_time() + 0.1);

    node_.subscribe_stop([this](const libbitcoin::code& ec) { running_ = false; });
    std::thread([this]() { this->run(); }).detach();
    return true;
}

void WsPushServ::run() {
    bc::log::info(NAME) << "WsPushServ listen on " << node_.server_settings().ws_stream_listen;
    
    node_.subscribe_transaction_pool(
        std::bind(&WsPushServ::handle_transaction,
            this, _1, _2, _3));
    
    running_ = true;
    while (running_)
    {
        mg_mgr_poll(&mgr_, 1000);
    }
}

bool WsPushServ::handle_transaction(const bc::code& ec, const index_list&, bc::message::transaction_message::ptr tx)
{
    if (!running_)
        return false;
    if (ec)
    {
        bc::log::info(NAME)
            << "Failure handling new transaction: " << ec.message();
        return true;
    }

    publish_transaction(*tx);
    return true;
}

void WsPushServ::publish_transaction(const bc::chain::transaction& tx)
{
    bc::message::transaction_message tx_msg(tx);
    std::stringstream otx;
    tx_msg.to_data(bc::message::version::level::maximum, otx);
    bc::log::info(NAME) << "publish TX: " << otx.str();
}

void WsPushServ::on_ws_handshake_req_handler(struct mg_connection& nc, http_message& msg)
{
    if (memcmp(msg.uri.p, "/ws", 3) != 0 || (msg.uri.len > 3 && msg.uri.p[3] != '/')) {
        nc.flags |= MG_F_SEND_AND_CLOSE;
    }
}

void WsPushServ::on_ws_handshake_done_handler(struct mg_connection& nc)
{
    //mg_set_timer(&nc, mg_time() + 1);
}

// {event: "subscribe", channel: "tx.monitor", "addresses":[]}
void WsPushServ::on_ws_frame_handler(struct mg_connection& nc, websocket_message& msg)
{
    struct mg_str d = { (char *)msg.data, msg.size };
    bc::log::info(NAME) << std::string(d.p, d.len);
    mg_send_websocket_frame(&nc, WEBSOCKET_OP_TEXT, msg.data, msg.size);
}

void WsPushServ::on_ws_ctrlf_handler(struct mg_connection& nc, websocket_message& msg)
{
}

void WsPushServ::on_timer_handler(struct mg_connection& nc)
{
    if (nc.flags & MG_F_USER_1)
    {
        mg_set_timer(&nc, mg_time() + 0.1);
        std::thread([this](){ mg_broadcast(&mgr_, ev_broadcast, "b", 1); }).detach();        
    }
}

void WsPushServ::on_close_handler(struct mg_connection& nc)
{
    if (nc.flags & MG_F_IS_WEBSOCKET)
    {

    }
}

void WsPushServ::on_broadcast(struct mg_connection& nc, const struct mg_str& msg)
{
    if (nc.flags & MG_F_USER_1)
        return;

    struct mg_connection *c;
    char buf[500];
    char addr[32];
    static const char* ping = "{\"event\":\"ping\"}";

    mg_sock_addr_to_str(&nc.sa, addr, sizeof(addr), MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);
    snprintf(buf, sizeof(buf), "%s %s", addr, ping);

    mg_send_websocket_frame(&nc, WEBSOCKET_OP_TEXT, ping, strlen(ping));
}

void WsPushServ::ev_broadcast(struct mg_connection *nc, int ev, void *ev_data)
{
    auto* self = dynamic_cast<WsPushServ*>(static_cast<WsPushServ*>(nc->mgr->user_data));
    if (!self)
        return;
    struct mg_str d = { (char *)ev_data, 1 };
    self->on_broadcast(*nc, d);
}

void WsPushServ::ev_handler(struct mg_connection *nc, int ev, void *ev_data)
{
    auto* self = dynamic_cast<WsPushServ*>(static_cast<WsPushServ*>(nc->mgr->user_data));
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
    }
}

}