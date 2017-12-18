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
#include <jsoncpp/json/json.h>
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
    
    /* *******************************************
     * application/json
     * {"method":"xxx", "params":""}
     * ******************************************/
    if (uri() == "/rpc" || uri() == "/rpc/")
    {
        Json::Reader reader;
        Json::Value root;
        const char* begin = body().data();
        const char* end = body().data() + body().size();
        reader.parse(begin, end, root);
        if (!reader.parse(begin, end, root) || !root.isObject()) {
            stringstream ss;
            ss << "parse request error, "
                << reader.getFormattedErrorMessages();
            throw std::runtime_error(ss.str());
            return ;
        }

        if (root["method"].isString())
            vargv_.emplace_back(root["method"].asString());

        if (root["params"].isArray() && (root["params"].size() > 0))
        {
            for (auto& param : root["params"]) {
                if (param.isString())
                    vargv_.emplace_back(param.asString());
            }
        }

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
