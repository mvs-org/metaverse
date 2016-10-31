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

void WebsocketMessage::data_to_arg()noexcept{
    Tokeniser<' '> args;
    args.reset(+*impl_);

    do {
        if(args.top().front() != ' '){
            vargv_.push_back({args.top().data(), args.top().size()});
            argc_++;
        }
        args.pop();
    }while(!args.empty());

    int i = 0;
    for(auto& iter : vargv_){
        argv_[i++] = iter.c_str();
    }

    //for(i = 0; i < argc_;  i++){
    //    bc::log::debug(LOG_HTTP)<<"argc_ "<<i<<":"<<argv_[i];
    //}
    bc::log::debug(LOG_HTTP)<<"ws got "<<argc_<<" paramters";
}

} // mg
} // http
