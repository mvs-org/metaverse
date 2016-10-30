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

#include "mvs/http/Exception_error.hpp"

#include "mongoose/mongoose.h"

#include <mvs/http/String.hpp>

/**
 * @addtogroup Web
 * @{
 */

namespace http {
namespace mg {

inline std::string_view operator+(const mg_str& str) noexcept
{
  return {str.p, str.len};
}

class HttpMessage {
public:
    HttpMessage(http_message* impl) noexcept : impl_{impl} {}
    ~HttpMessage() noexcept = default;
    
    // Copy.
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
    
private:
    http_message* impl_;
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

protected:
    Mgr() noexcept { mg_mgr_init(&mgr_, this); }
    ~Mgr() noexcept { mg_mgr_free(&mgr_); }

 private:
    static void handler(mg_connection* conn, int event, void* data)
    {
       http_message* hm = static_cast<http_message*>(data);
       auto* self = static_cast<DerivedT*>(conn->user_data);

       switch (event) {
       case MG_EV_CLOSE:
            conn->user_data = nullptr;
            break;
       case MG_EV_HTTP_REQUEST:
            if (mg_ncasecmp((&hm->uri)->p, "/mvsrpc", 7u) == 0) {
                self->httpRequest(*conn, hm);
            }else{
                self->httpStatic(*conn, hm);
            }
            break;
       }
    }

  mg_mgr mgr_;
};

} // mg
} // http

/** @} */

#endif // MVSD_MONGOOSE_HPP
