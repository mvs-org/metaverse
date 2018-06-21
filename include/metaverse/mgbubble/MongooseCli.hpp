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
#ifndef MVSD_MONGOOSECLI_HPP
#define MVSD_MONGOOSECLI_HPP

#include <iostream>
#include <functional>
#include "mongoose/mongoose.h"

namespace mgbubble {
namespace cli {

typedef std::function<void(const http_message*)> reply_handler;

template <typename DerivedT>
class MgrCli {
public:
    // Copy.
    MgrCli(const MgrCli&) = delete;
    MgrCli& operator=(const MgrCli&) = delete;

    // Move.
    MgrCli(MgrCli&&) = delete;
    MgrCli& operator=(MgrCli&&) = delete;

    inline time_t poll(int milli) { return mg_mgr_poll(&mgr_, milli); }

protected:
    MgrCli() noexcept { mg_mgr_init(&mgr_, this); }
    ~MgrCli() noexcept { mg_mgr_free(&mgr_); }

    static void ev_handler(mg_connection* nc, int ev, void *ev_data) {
       auto* hm = static_cast<http_message*>(ev_data);
       auto* self = static_cast<DerivedT*>(nc->user_data);//this

      switch (ev) {
        case MG_EV_CONNECT:
            if (* (int *) ev_data != 0) {
                fprintf(stderr, "connect[%s] failed: %s\n", 
                        self->get_url().c_str(), strerror(* (int *) ev_data));
                self->exit();
            }
            break;
        case MG_EV_HTTP_REPLY:
            nc->flags |= MG_F_CLOSE_IMMEDIATELY;
            self->reply(hm);
            self->exit();
            break;
        default:
            break;
      }
    }

    mg_mgr mgr_;
};

class HttpReq : public MgrCli<HttpReq>
{
public:
    explicit HttpReq(const std::string& url, int milli, reply_handler&& oreply)
        :url_(url), reply(oreply){
            memset(&opts_, 0x00, sizeof(opts_));
            opts_.user_data = reinterpret_cast<void*>(this);

            if (milli > 0) 
                milli_ = milli;
        }
    ~HttpReq() noexcept {}

    //void got_reply(http_message* msg) { reply(msg); }

    const std::string& get_url(){ return url_; }
    void set_url(const std::string& other){ url_ = other; }
    void set_url(std::string&& other){ url_ = other; }
    void exit(){ exit_ = true; }
    void reset(){ exit_ = false; }

    void get() { 
        conn_ = mg_connect_http_opt(&mgr_, ev_handler, opts_, url_.c_str(), NULL, NULL); 
        while (!exit_){
            poll(milli_);
        }
    }
    void post(std::string&& data) { post(data); }
    void post(const std::string& data) { 
        conn_ = mg_connect_http_opt(&mgr_, ev_handler, opts_, url_.c_str(), NULL, data.c_str()); 
        while (!exit_){
            poll(milli_);
        }
    }
    void post(std::string&& header, std::string&& data) { post(header, data); }
    void post(const std::string& header, const std::string& data) { 
        conn_ = mg_connect_http_opt(&mgr_, ev_handler, opts_, url_.c_str(), header.c_str(), data.c_str()); 
        while (!exit_){
            poll(milli_);
        }
    }

    reply_handler reply;
private:
    int  milli_{3000};
    bool exit_{false};
    mg_connect_opts opts_;
    mg_connection* conn_{nullptr};
    std::string url_;
};

} // mg
} // http

/** @} */

#endif // MVSD_MONGOOSECLI_HPP
