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

#define SESSION_COOKIE_NAME "mvss"

inline std::string_view operator+(const mg_str& str) noexcept
{
    return {str.p, str.len};
}

inline std::string_view operator+(const websocket_message& msg) noexcept
{
    return {reinterpret_cast<char*>(msg.data), msg.size};
}

class ToCommandArg{
public:
    auto argv() const noexcept { return argv_; }
    auto argc() const noexcept { return argc_; }
    void add_arg(std::string&& outside);

    static const int max_paramters{32};
protected:

    virtual void data_to_arg() noexcept = 0;
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
    HttpMessage(const HttpMessage&) noexcept = default;
    HttpMessage& operator=(const HttpMessage&) noexcept = default;
    
    // Move.
    HttpMessage(HttpMessage&&) noexcept = default;
    HttpMessage& operator=(HttpMessage&&) noexcept = default;
    
    auto get() const noexcept { return impl_; }
    auto method() const noexcept { return +impl_->method; }
    auto uri() const noexcept { return +impl_->uri; }
    auto proto() const noexcept { return +impl_->proto; }
    auto queryString() const noexcept { return +impl_->query_string; }
    auto header(const char* name) const noexcept
    {
      auto* val = mg_get_http_header(impl_, name);
      return val ? +*val : std::string_view{};
    }
    auto body() const noexcept { return +impl_->body; }

    void data_to_arg() noexcept override;
    
private:

    http_message* impl_;
};

class WebsocketMessage:public ToCommandArg { // connect to bx command-tool
public:
    WebsocketMessage(websocket_message* impl) noexcept : impl_{impl} {}
    ~WebsocketMessage() noexcept = default;
    
    // Copy.
    WebsocketMessage(const WebsocketMessage&) noexcept = default;
    WebsocketMessage& operator=(const WebsocketMessage&) noexcept = default;
    
    // Move.
    WebsocketMessage(WebsocketMessage&&) noexcept = default;
    WebsocketMessage& operator=(WebsocketMessage&&) noexcept = default;
    
    auto get() const noexcept { return impl_; }
    auto data() const noexcept { return reinterpret_cast<char*>(impl_->data); }
    auto size() const noexcept { return impl_->size; }
   
    void data_to_arg() noexcept override;
private:
    websocket_message* impl_;
};

struct Session{
        Session() = default;
        Session(uint64_t a1, double a2, double a3, 
                std::string&& a4, uint16_t a5):
            id(a1), created(a2), last_used(a3), user(a4), state(a5){}
        ~Session() = default;

        uint64_t        id;
        double          created;
        double          last_used;
        std::string     user;
        uint16_t        state;
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
           char user[50], pass[50];
           auto ul = mg_get_http_var(&hm->body, "user", user, sizeof(user));
           auto pl = mg_get_http_var(&hm->body, "pass", pass, sizeof(pass));
           if (ul > 0 && pl > 0) {
              if(!self->user_auth({user, std::strlen(user)}, {pass, std::strlen(pass)})){
                mg_printf(conn, "HTTP/1.0 403 Unauthorized\r\n\r\nWrong password.\r\n");
              }

              auto ret = self->push_session({user, std::strlen(user)}, hm);
              std::ostringstream shead;
              shead<<"Set-Cookie: " SESSION_COOKIE_NAME "="<<ret->id<<"; path=/";
              mg_http_send_redirect(conn, 302, mg_mk_str("/"), mg_mk_str(shead.str().c_str()));

           } else {
              mg_printf(conn, "HTTP/1.0 400 Bad Request\r\n\r\nuser, pass required.\r\n");
           }
       }
       conn->flags |= MG_F_SEND_AND_CLOSE;
    }

    static void logout_handler(mg_connection* conn, int ev, void* data){
       http_message* hm = static_cast<http_message*>(data);
       auto* self = static_cast<DerivedT*>(conn->user_data);
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
            }
            // register call
            if (mg_ncasecmp((&hm->uri)->p, "/api/getnewaccount", 18u) == 0) {
                self->httpRequest(*conn, hm);
                break;
            }

            // login required for other calls
            if (!self->get_from_session_list(hm)) {
                mg_http_send_redirect(conn, 302, mg_mk_str("/login.html"),
                        mg_mk_str(nullptr));
                conn->flags |= MG_F_SEND_AND_CLOSE;
                break;
            }

            // logined, http request process
            if (mg_ncasecmp((&hm->uri)->p, "/api", 4u) == 0) {
                self->httpRequest(*conn, hm);
            }else{
                self->httpStatic(*conn, hm);
                conn->flags |= MG_F_SEND_AND_CLOSE;
            }
            break;
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
            mg_set_timer(conn, mg_time() + self->session_check_interval);
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
