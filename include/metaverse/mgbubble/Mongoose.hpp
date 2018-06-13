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
#ifndef MVSD_MONGOOSE_HPP
#define MVSD_MONGOOSE_HPP

#include <vector>
#include <metaverse/mgbubble/utility/Queue.hpp>
#include <metaverse/mgbubble/utility/String.hpp>
#include <metaverse/mgbubble/exception/Error.hpp>
#include <metaverse/explorer/dispatch.hpp>
#include "mongoose/mongoose.h"
/**
 * @addtogroup Web
 * @{
 */

namespace mgbubble {

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

    void add_arg(std::string&& outside);

    static const int max_paramters{208};
protected:

    virtual void data_to_arg(uint8_t api_version) = 0;
    const char* argv_[max_paramters]{nullptr};
    int argc_{0};

    std::vector<std::string> vargv_;
};

class HttpMessage : public ToCommandArg{
public:
    HttpMessage(http_message* impl) noexcept : impl_{impl}, jsonrpc_id_(-1){}
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

    const int64_t jsonrpc_id() const noexcept { return jsonrpc_id_; }

    void data_to_arg(uint8_t rpc_version) override;

private:
    int64_t jsonrpc_id_;
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

    void data_to_arg(uint8_t api_version = 1) override;
private:
    websocket_message* impl_;
};

class MgEvent : public std::enable_shared_from_this<MgEvent> {
public:
    explicit MgEvent(const std::function<void(uint64_t)>&& handler)
        :callback_(std::move(handler))
    {}

    MgEvent* hook()
    {
        self_ = this->shared_from_this();
        return this;
    }

    void unhook()
    {
        self_.reset();
    }

    virtual void operator()(uint64_t id)
    {
        callback_(id);
        self_.reset();
    }

private:
    std::shared_ptr<MgEvent> self_;

    // called on mongoose thread
    std::function<void(uint64_t id)> callback_;
};

} // http

/** @} */

#endif // MVSD_MONGOOSE_HPP
