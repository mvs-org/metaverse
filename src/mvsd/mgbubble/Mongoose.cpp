/*
 * Copyright (c) 2016-2021 metaverse core developers (see MVS-AUTHORS).
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
#include <metaverse/explorer/extensions/exception.hpp>

namespace mgbubble {

void HttpMessage::data_to_arg(uint8_t rpc_version) {

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

    Json::Reader reader;
    Json::Value root;
    const char* begin = body().data();
    const char* end = body().data() + body().size();
    if (!reader.parse(begin, end, root) || !root.isObject()) {
        throw libbitcoin::explorer::jsonrpc_parse_error();
    }

    if (root["method"].isString()) {
        vargv_.emplace_back(root["method"].asString());
    }

    if (root.isMember("params") && !root["params"].isArray()) {
        throw libbitcoin::explorer::jsonrpc_invalid_params();
    }

    if (rpc_version == 1) {
        /* ***************** /rpc **********************
         * application/json
         * {"method":"xxx", "params":["p1","p2"]}
         * ******************************************/
        for (auto& param : root["params"]) {
            if (!param.isObject())
                vargv_.emplace_back(param.asString());
        }
    } else {
        /* ***************** /rpc/v2 or /rpc/v3 **********************
         * application/json
         * {
         *  "method":"xxx",
         *  "params":[
         *      {
         *          k1:v1,  ==> Command Option
         *          k2:v2
         *      },
         *      "p1",  ==> Command Argument
         *      "p2"
         *      ]
         *  }
         * ******************************************/

        const std::vector<std::string> api20_ver_list = {"2.0", "3.0"};
        auto checkAPIVer = [](const std::vector<std::string> &api_ver_list, const std::string &rpc_version){
            return find(api_ver_list.begin(), api_ver_list.end(), rpc_version) != api_ver_list.end();
        };

        if (!checkAPIVer(api20_ver_list, root["jsonrpc"].asString())) {
            throw libbitcoin::explorer::jsonrpc_invalid_request();
        }

        if (root["id"].isString()) {
            jsonrpc_id_ = std::stol(root["id"].asString());
        } else {
            jsonrpc_id_ = root["id"].asInt64();
        }

        // push options
        for (auto& param : root["params"]) {
            if (param.isObject()) {
                for (auto& key : param.getMemberNames()) {
                    if (!param[key].empty()) {

                        if (!param[key].isArray()) {
                            // --option
                            vargv_.emplace_back("--" + key);
                            // value
                            vargv_.emplace_back(param[key].asString());
                        } else  {
                            for (auto& member : param[key]) {
                                // --option
                                vargv_.emplace_back("--" + key);
                                // value
                                vargv_.emplace_back(member.asString());
                            }
                        }

                    } else {
                        // --option
                        vargv_.emplace_back("--" + key);
                    }
                }
                break;
            }
        }

        // push arguments at last
        for (auto& param : root["params"]) {
            if (!param.isObject()){
                vargv_.emplace_back(param.asString());
            }
        }
    }

    vargv_to_argv();
}

void WebsocketMessage::data_to_arg(uint8_t api_version) {
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
