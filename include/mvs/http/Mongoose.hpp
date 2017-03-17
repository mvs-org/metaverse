/*
 * The NewReality Blockchain Project.
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
#ifndef MVSD_MONGOOSE_HPP
#define MVSD_MONGOOSE_HPP

#include <bitcoin/explorer/dispatch.hpp>
#include "mongoose/mongoose.h"

#include <mvs/http/String.hpp>
#include <mvs/http/Exception_error.hpp>

/**
 * @addtogroup Web
 * @{
 */

namespace http {
namespace mg {

#define SESSION_COOKIE_NAME "2iBXdhW9rQxbnDdNQk9KdjiytM9X"

inline string_view operator+(const mg_str& str) noexcept
{
    return {str.p, str.len};
}

inline string_view operator+(const websocket_message& msg) noexcept
{
    return {reinterpret_cast<char*>(msg.data), msg.size};
}

class ToCommandArg{
public:
    auto argv() const noexcept { return argv_; }
    auto argc() const noexcept { return argc_; }
    const auto& get_command() const { 
        if(!vargv_.empty()) 
            return vargv_[0]; 
        throw std::logic_error{"no command found"};
    }
    bool is_miner_command() const {
        auto cmd = get_command();
         if (cmd == "getwork" ||
             cmd == "submitwork" ||
             cmd == "start" ||
             cmd == "stop"  ||
             cmd == "setminingaccount" ||
             cmd == "getmininginfo" ||
             cmd == "fetchheaderext"){
            return true;
        }
        return false;
    }
    void add_arg(std::string&& outside);

    static const int max_paramters{32};
protected:

    virtual void data_to_arg() = 0;
    const char* argv_[max_paramters]{{nullptr}};
    int argc_{0};

    std::vector<std::string> vargv_;
};


class HttpMessage : public ToCommandArg{
public:
    HttpMessage(http_message* impl) noexcept : impl_{impl} {}
    ~HttpMessage() noexcept = default;
    
    // Copy.
    // http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#1778
    HttpMessage(const HttpMessage&) = default;
    HttpMessage& operator=(const HttpMessage&) = default;
    
    // Move.
    HttpMessage(HttpMessage&&) = default;
    HttpMessage& operator=(HttpMessage&&) = default;
    
    auto get() const noexcept { return impl_; }
    auto method() const noexcept { return +impl_->method; }
    auto uri() const noexcept { return +impl_->uri; }
    auto proto() const noexcept { return +impl_->proto; }
    auto queryString() const noexcept { return +impl_->query_string; }
    auto header(const char* name) const noexcept
    {
      auto* val = mg_get_http_header(impl_, name);
      return val ? +*val : string_view{};
    }
    auto body() const noexcept { return +impl_->body; }

    void data_to_arg() override;
    
private:

    http_message* impl_;
};

class WebsocketMessage:public ToCommandArg { // connect to bx command-tool
public:
    WebsocketMessage(websocket_message* impl) noexcept : impl_{impl} {}
    ~WebsocketMessage() noexcept = default;
    
    // Copy.
    WebsocketMessage(const WebsocketMessage&) = default;
    WebsocketMessage& operator=(const WebsocketMessage&) = default;
    
    // Move.
    WebsocketMessage(WebsocketMessage&&) = default;
    WebsocketMessage& operator=(WebsocketMessage&&) = default;
    
    auto get() const noexcept { return impl_; }
    auto data() const noexcept { return reinterpret_cast<char*>(impl_->data); }
    auto size() const noexcept { return impl_->size; }
   
    void data_to_arg() override;
private:
    websocket_message* impl_;
};

struct Session{
        Session() = default;
        Session(uint64_t a1, double a2, double a3, 
                std::string&& a4, std::string a5):
            id(a1), created(a2), last_used(a3), user(a4), pass(a5){}
        ~Session() = default;

        uint64_t        id;
        double          created;
        double          last_used;
        std::string     user;
        std::string     pass;
};

template <typename DerivedT>
class Mgr {
public:
    // Copy.
    Mgr(const Mgr&) = delete;
    Mgr& operator=(const Mgr&) = delete;

    // Move.
    Mgr(Mgr&&) = delete;
    Mgr& operator=(Mgr&&) = delete;

    mg_connection& bind(const char* addr)
    {
      auto* conn = mg_bind(&mgr_, addr, handler);
      if (!conn)
        throw Error{"mg_bind() failed"};
      conn->user_data = this;
      return *conn;
    }
    time_t poll(int milli) { return mg_mgr_poll(&mgr_, milli); }

    // session control
    static void login_handler(mg_connection* conn, int ev, void* data){
       http_message* hm = static_cast<http_message*>(data);
       auto* self = static_cast<DerivedT*>(conn->user_data);

       if (mg_vcmp(&hm->method, "POST") != 0) {
           mg_serve_http(conn, hm, self->get_httpoptions());
       }else{
              if ( self->user_auth(*conn, hm) ){
                  std::ostringstream shead;
                  auto ret = self->push_session(hm);
                  shead<<"Set-Cookie: " SESSION_COOKIE_NAME "="<<ret->id<<"; path=/";
                  mg_http_send_redirect(conn, 302, mg_mk_str("/"), mg_mk_str(shead.str().c_str()));
              }
       }
       conn->flags |= MG_F_SEND_AND_CLOSE;
    }

    static void logout_handler(mg_connection* conn, int ev, void* data){
       http_message* hm = static_cast<http_message*>(data);
       auto* self = static_cast<DerivedT*>(conn->user_data);

       std::ostringstream shead;
       shead<<"Set-Cookie: " SESSION_COOKIE_NAME "=";
       mg_http_send_redirect(conn, 302, mg_mk_str("/login.html"), mg_mk_str(shead.str().c_str()));
       self->remove_from_session_list(hm);

       conn->flags |= MG_F_SEND_AND_CLOSE;
    }

    constexpr static const double session_check_interval = 5.0;

protected:
    Mgr() noexcept { mg_mgr_init(&mgr_, this); }
    ~Mgr() noexcept { mg_mgr_free(&mgr_); }

private:

    static void handler(mg_connection* conn, int event, void* data)
    {
       http_message* hm = static_cast<http_message*>(data);
       websocket_message* ws = static_cast<websocket_message*>(data);
       auto* self = static_cast<DerivedT*>(conn->user_data);

       switch (event) {
       case MG_EV_CLOSE:{
            if (conn->flags & MG_F_IS_WEBSOCKET) {
                //self->websocketBroadcast(*conn, "left", 4);
            }else{
                conn->user_data = nullptr;
            }
            break;
        }
       case MG_EV_HTTP_REQUEST:{
            // rpc call
            if (mg_ncasecmp((&hm->uri)->p, "/rpc", 4u) == 0){
                self->httpRpcRequest(*conn, hm);
                break;
            }else{
                self->httpStatic(*conn, hm);
                conn->flags |= MG_F_SEND_AND_CLOSE;
                break;
            }
        }
        case MG_EV_WEBSOCKET_HANDSHAKE_DONE:{
            self->websocketSend(conn, "connected", 9);
            break;
        }
        case MG_EV_WEBSOCKET_FRAME:{
            self->websocketSend(*conn, ws);
            break;
        }
        case MG_EV_SSI_CALL:{
        }
        case MG_EV_TIMER:{
            self->check_sessions();
            mg_set_timer(conn, mg_time() + session_check_interval);
            break;
        }
       }// switch
    }// handler

    mg_mgr mgr_;
};

} // mg
} // http

/** @} */

#endif // MVSD_MONGOOSE_HPP
