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
#include "mvs/http/Mongoose.hpp"
#include "json/minijson_reader.hpp"
#include "mvs/http/Tokeniser.hpp"

namespace http {
namespace mg {

void HttpMessage::data_to_arg() noexcept {

    /* *******************************************
     * application/json
     * {"method":"xxx", "params":""}
     * ******************************************/
    if (uri() == "/rpc" or uri() == "/rpc/")
    {
        minijson::const_buffer_context ctx(body().data(), body().size());
        minijson::parse_object(ctx, 
            [&](const char* key, minijson::value value){
            minijson::dispatch (key)
            <<"method">> [&]{ mvs_method_ = value.as_string(); }
            <<"params">> [&]{ mvs_params_ = value.as_string(); }
            <<minijson::any>> [&]{ minijson::ignore(ctx); };
        });

    }

    /* *******************************************
     * application/x-www-form-urlencoded
     * method=xxx&params=xxx
     * ******************************************/
    if (uri().substr(0,4) == "/api")
    {
        std::array<char, 4096> params{0x00};
        mg_get_http_var(&impl_->body, "params", params.begin(), params.max_size());
        mvs_params_ = std::string(params.data(), std::strlen(params.data()));
    }
}

void WebsocketMessage::data_to_arg() noexcept {
    Tokeniser<' '> args;
    args.reset(+*impl_);

    // store args from ws message
    do {
        //skip spaces
        if(args.top().front() != ' '){
            mvs_method_ = std::string({args.top().data(), args.top().size()});
            break;
        }
        args.pop();
    }while(!args.empty());

    mvs_params_ = std::string{reinterpret_cast<char*>(impl_->data + mvs_method_.size()), 
            impl_->size - mvs_method_.size()};
}

} // mg
} // http
