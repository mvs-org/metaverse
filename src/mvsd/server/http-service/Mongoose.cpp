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

namespace http {
namespace mg {

void HttpMessage::data_to_arg() noexcept {

    bc::log::debug(LOG_HTTP)<<"uri "<<uri();

    auto convert_to_arg = [this](std::string_view data){
        Tokeniser<' '> args;
        args.reset(data);

        // store args from ws message
        do {
            //skip spaces
            if(args.top().front() != ' '){
                this->vargv_.push_back({args.top().data(), args.top().size()});
                this->argc_++;
            }
            args.pop();
        }while(!args.empty());

        // convert to char** argv
        int i = 0;
        for(auto& iter : this->vargv_){
            if (i >= max_paramters){
                break;
            }
            this->argv_[i++] = iter.c_str();
        }

    };

    /* application/json
     * {"method":"xxx", "params":[]"}
     */
    if (uri() == "/rpc")
    {
    }

    /* application/x-www-form-urlencoded
     * method=xxx&params=xxx
     */
    if (uri() == "/api")
    {
        char n1[128]="";
        char n2[128]="";

        mg_get_http_var(&impl_->body, "method", n1, sizeof(n1));
        mg_get_http_var(&impl_->body, "params", n2, sizeof(n2));
        convert_to_arg({n1, 128});
    }

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

    bc::log::debug(LOG_HTTP)<<"ws got cmd:["<<argv_[0]<<"],paramters["<<argc_-1<<"]";
}

} // mg
} // http
