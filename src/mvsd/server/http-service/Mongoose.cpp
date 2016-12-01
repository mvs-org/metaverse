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
#include "mvs/http/Tokeniser.hpp"
#include "json/minijson_reader.hpp"

namespace http {
namespace mg {

void HttpMessage::data_to_arg() noexcept {

    auto convert = [this](std::string_view method, std::string_view pramas){

        if (!method.empty()){
            this->vargv_.push_back({method.data(), method.size()});
            this->argc_++;
        }

        if (!pramas.empty()){
            Tokeniser<' '> args;
            args.reset(pramas);

            // store args from ws message
            do {
                //skip spaces
                if(args.top().front() != ' '){
                    this->vargv_.push_back({args.top().data(), args.top().size()});
                    this->argc_++;
                }
                args.pop();
            }while(!args.empty());
        }

        // convert to char** argv
        int i = 0;
        for(auto& iter : this->vargv_){
            if (i >= max_paramters){
                break;
            }
            this->argv_[i++] = iter.c_str();
        }

    };

    /* *******************************************
     * application/json
     * {"method":"xxx", "params":""}
     * ******************************************/
    if (uri() == "/rpc" or uri() == "/rpc/")
    {
        std::string method, params;
        minijson::const_buffer_context ctx(body().data(), body().size());
        minijson::parse_object(ctx, 
            [&](const char* key, minijson::value value){
            minijson::dispatch (key)
            <<"method">> [&]{ method = value.as_string(); }
            <<"params">> [&]{ params = value.as_string(); }
            <<minijson::any>> [&]{ minijson::ignore(ctx); };
        });

        convert({method.c_str(), method.size()}, 
                {params.c_str(), params.size()});
    }

    /* *******************************************
     * application/x-www-form-urlencoded
     * method=xxx&params=xxx
     * ******************************************/
    if (uri().substr(0,4) == "/api")
    {
        std::array<char, 4096> params{0x00};
        mg_get_http_var(&impl_->body, "params", params.begin(), params.max_size());

        convert({nullptr, 0u}, 
                {params.data(), std::strlen(params.data())});
    }

    bc::log::debug(LOG_HTTP)<<"Got:["<<argv_[0]<<"],params count["<<argc_-1<<"]";
}

void WebsocketMessage::data_to_arg() noexcept {
    Tokeniser<' '> args;
    args.reset(+*impl_);

    // store args from ws message
    do {
        //skip spaces
        if(args.top().front() != ' '){
            vargv_.push_back({args.top().data(), args.top().size()});
            argc_++;
        }
        args.pop();
    }while(!args.empty());

    // convert to char** argv
    int i = 0;
    for(auto& iter : vargv_){
        if (i >= max_paramters){
            break;
        }
        argv_[i++] = iter.c_str();
    }

    bc::log::debug(LOG_HTTP)<<"Got:["<<argv_[0]<<"],params count["<<argc_-1<<"]";
}

void ToCommandArg::add_arg(std::string&& outside)
{
    vargv_.push_back(outside); 
    argc_++; 
}

} // mg
} // http
