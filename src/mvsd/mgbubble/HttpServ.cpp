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
#include <exception>
#include <functional> //hash

#include <metaverse/mgbubble/HttpServ.hpp>
#include <metaverse/mgbubble/exception/Instances.hpp>
#include <metaverse/mgbubble/utility/Stream_buf.hpp>

#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/server/server_node.hpp>

namespace mgbubble{

thread_local OStream HttpServ::out_;
thread_local Tokeniser<'/'> HttpServ::uri_;
thread_local int HttpServ::state_ = 0;

void HttpServ::reset(HttpMessage& data) noexcept
{
    state_ = 0;

    const auto method = data.method();
    if (method == "GET") {
      state_ |= MethodGet;
    } else if (method == "POST") {
      state_ |= MethodPost;
    } else if (method == "PUT") {
      state_ |= MethodPut;
    } else if (method == "DELETE") {
      state_ |= MethodDelete;
    }

    auto uri = data.uri();
    // Remove leading slash.
    if (uri.front() == '/') {
      uri.remove_prefix(1);
    }
    uri_.reset(uri);
}

void HttpServ::rpc_request(mg_connection& nc, HttpMessage data, uint8_t rpc_version)
{
    reset(data);

    StreamBuf buf{ nc.send_mbuf };
    out_.rdbuf(&buf);
    out_.reset(200, "OK");
    try {
        data.data_to_arg(rpc_version);

        std::stringstream sout;
        std::istringstream sin;

        console_result retcode = explorer::dispatch_command(data.argc(), const_cast<const char**>(data.argv()),
            sin, sout, sout, node_, rpc_version);
        if (retcode != console_result::okay) {
            throw explorer::command_params_exception(sout.str());
        }

        explorer::relay_exception(sout);

        out_ << sout.str();
    }
    catch (const libbitcoin::explorer::explorer_exception& e) {
        out_ << e;
    }
    catch (const std::exception& e) {
        libbitcoin::explorer::explorer_exception ex(1000, e.what());
        out_ << ex;
    }

    out_.setContentLength();
}

void HttpServ::ws_request(mg_connection& nc, WebsocketMessage ws)
{
    std::stringstream sout;
    std::istringstream sin;
    try{
        ws.data_to_arg();

        console_result retcode = explorer::dispatch_command(ws.argc(), const_cast<const char**>(ws.argv()), sin, sout, sout, node_);
        if (retcode != console_result::okay) {
            throw explorer::command_params_exception(sout.str());
        }
        explorer::relay_exception(sout);
    } catch(libbitcoin::explorer::explorer_exception ex) {
        sout << ex;
    } catch(std::exception& e) {
        libbitcoin::explorer::explorer_exception ex(1000, e.what());
        sout << ex;
    } catch(...) {
        log::error(LOG_HTTP)<<sout.rdbuf();
        libbitcoin::explorer::explorer_exception ex(1001,"fatal error");
        sout << ex;
    }

    send_frame(nc, sout.str());
}

bool HttpServ::start()
{
    if (!attach_notify())
        return false;
    return base::start();
}

void HttpServ::spawn_to_mongoose(const std::function<void(uint64_t)>&& handler)
{
    auto msg = std::make_shared<MgEvent>(std::move(handler));
    struct mg_event ev { msg->hook() };
    if (!notify(ev))
        msg->unhook();
}

void HttpServ::run() {
    log::info(LOG_HTTP) << "Http Service listen on " << node_.server_settings().websocket_listen;

    node_.subscribe_stop([this](const libbitcoin::code& ec) { stop(); });

    base::run();

    log::info(LOG_HTTP) << "Http Service Stopped.";
}

void HttpServ::on_http_req_handler(struct mg_connection& nc, http_message& msg)
{
    if ((mg_ncasecmp(msg.uri.p, "/rpc/v2", 7) == 0) || (mg_ncasecmp(msg.uri.p, "/rpc/v2/", 8) == 0)) {
        rpc_request(nc, HttpMessage(&msg), 2); // v2 rpc
    }
    else if ((mg_ncasecmp(msg.uri.p, "/rpc", 4) == 0) || (mg_ncasecmp(msg.uri.p, "/rpc/", 5) == 0)) {
        rpc_request(nc, HttpMessage(&msg), 1); //v1 rpc
    } else {
        std::shared_ptr<struct mg_connection> con(&nc, [](struct mg_connection* ptr) { (void)(ptr); });
        serve_http_static(nc, msg);
    }
}

void HttpServ::on_notify_handler(struct mg_connection& nc, struct mg_event& ev)
{
    static uint64_t api_call_counter = 0;

    if (ev.data == nullptr)
        return;

    auto& msg = *(MgEvent*)ev.data;
    msg(++api_call_counter);
}

void HttpServ::on_ws_handshake_done_handler(struct mg_connection& nc)
{
    std::shared_ptr<struct mg_connection> con(&nc, [](struct mg_connection* ptr) { (void)(ptr); });
    send_frame(nc, "connected", 9);
}

void HttpServ::on_ws_frame_handler(struct mg_connection& nc, websocket_message& msg)
{
    std::istringstream iss;
    iss.str(std::string((const char*)msg.data, msg.size));
    ws_request(nc, WebsocketMessage(&msg));
}

}// mgbubble

