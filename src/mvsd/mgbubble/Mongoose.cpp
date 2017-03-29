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
#include <cctype>
#include <json/minijson_reader.hpp>
#include <metaverse/mgbubble/Mongoose.hpp>
#include <metaverse/mgbubble/utility/Tokeniser.hpp>

namespace mgbubble {

void HttpMessage::data_to_arg() {

    auto vargv_to_argv = [this]() {
        // convert to char** argv
        int i = 0;
        for(auto& iter : this->vargv_){
            if (i >= max_paramters){
                break;
            }
            this->argv_[i++] = iter.c_str();
        }
        argc_ = i;
    };

    auto convert = [this](string_view method, string_view pramas){

        if (!method.empty()){
            this->vargv_.push_back({method.data(), method.size()});
        }

        if (!pramas.empty()){
            Tokeniser<' '> args;
            args.reset(pramas);

            // store args from ws message
            do {
                //skip spaces
                if (args.top().front() == ' '){
                    args.pop();
                    continue;
                } else if (std::iscntrl(args.top().front())){
                    break;
                } else {
                    this->vargv_.push_back({args.top().data(), args.top().size()});
                    args.pop();
                }
            }while(!args.empty());
        }
    
    };

    /* *******************************************
     * application/json
     * {"method":"xxx", "params":""}
     * ******************************************/
    if (uri() == "/rpc" || uri() == "/rpc/")
    {

#if 0
        pt::ptree root;
        std::istringstream sin;
        sin.str({body().data(), body().size()});
        pt::read_json(sin, root);
        auto&& method = root.get<std::string>("method");
        if (method.size()){
            vargv_.push_back(method);
        }

        for (auto& each : root.get_child("params.")) {
            vargv_.push_back(each.second.data());
        }
#else
        minijson::const_buffer_context ctx(body().data(), body().size());
        minijson::parse_object(ctx, 
            [&, this](const char* key, minijson::value value){
            minijson::dispatch (key)
            <<"method">> [&,this]{ 
                   std::string&& method = value.as_string();
                   if (method.size()){
                       vargv_.insert(vargv_.begin(), method);
                    }
                }
            <<"params">> [&, this]{ 
                minijson::parse_array(ctx, [&](minijson::value v) {
                   std::string&& params = v.as_string();
                   if (params.size())
                       vargv_.push_back(params);
                });
             }
            <<minijson::any>> [&]{ minijson::ignore(ctx); };
        });
#endif

        vargv_to_argv();
    }

    /* *******************************************
     * application/x-www-form-urlencoded
     * method=xxx&params=xxx
     * ******************************************/
    if (uri().substr(0,4) == "/api")
    {
        std::array<char, 4096> params{0x00};
        auto len = mg_get_http_var(&impl_->body, "params", params.data(), params.max_size());

        convert({nullptr, 0u}, 
                {params.data(), static_cast<string_view::size_type>(len)});
        vargv_to_argv();
    }

}

void WebsocketMessage::data_to_arg() {
    Tokeniser<' '> args;
    args.reset(+*impl_);

    // store args from ws message
    do {
        //skip spaces
        if (args.top().front() == ' '){
            args.pop();
            continue;
        } else if (std::iscntrl(args.top().front())){
            break;
        } else {
            this->vargv_.push_back({args.top().data(), args.top().size()});
            args.pop();
        }
    }while(!args.empty());

    // convert to char** argv
    int i = 0;
    for(auto& iter : vargv_){
        if (i >= max_paramters){
            break;
        }
        argv_[i++] = iter.c_str();
    }
    argc_ = i;
}

void ToCommandArg::add_arg(std::string&& outside)
{
    vargv_.push_back(outside); 
    argc_++; 
}

} // mgbubble
