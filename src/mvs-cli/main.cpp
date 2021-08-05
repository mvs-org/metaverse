/**
 * Copyright (c) 2011-2021 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2021 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse-explorer.
 *
 * metaverse-explorer is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/bitcoin/utility/path.hpp>
#include <metaverse/bitcoin/unicode/ifstream.hpp>
#include <boost/program_options.hpp>
#include <jsoncpp/json/json.h>
#include <metaverse/mgbubble/MongooseCli.hpp>
#include <metaverse/bitcoin/unicode/unicode.hpp>

BC_USE_MVS_MAIN

/**
 * Invoke this program with the raw arguments provided on the command line.
 * All console input and output streams for the application originate here.
 * @param argc  The number of elements in the argv array.
 * @param argv  The array of arguments, including the process.
 * @return      The numeric result to return via console exit.
 */
using namespace mgbubble::cli;
namespace po = boost::program_options;

void my_impl(const http_message* hm)
{
    auto&& reply = std::string(hm->body.p, hm->body.len);
    Json::Reader reader;
    Json::Value root;
    if (reader.parse(reply, root) && root.isObject()) {
        if (root["error"]["code"].isInt() && root["error"]["code"].asInt() != 0) {
            bc::cout << root["error"].toStyledString();
        }
        else if (root["result"].isString()) {
            bc::cout << root["result"].asString() <<std::endl;
        }
        else if(root["result"].isArray() || root["result"].isObject()) {
            bc::cout << root["result"].toStyledString();
        }
        else {
            bc::cout << reply << std::endl;
        }
    }
    else {
        bc::cout << reply << std::endl;
    }
}

int bc::main(int argc, char* argv[])
{
    bc::set_utf8_stdout();
    auto work_path = bc::default_data_path();
    auto&& config_file = work_path / "mvs.conf";
    std::string default_rpc_version = "3";
    std::string url{"127.0.0.1:8820/rpc/v" + default_rpc_version};

    // use '-c file_name' to specify config file name
    if (argc > 1 && std::string(argv[1]) == "-c") {
        if (argc < 3 || std::string(argv[2]).empty()) {
            std::cout << "'-c' option must followed by a non-empty config file name."
                      << std::endl;
            return 0;
        }
        std::string file_name = argv[2];
        config_file = work_path / file_name;
        if (!boost::filesystem::exists(config_file)) {
            std::cout << "The specified config file '"
                      << config_file.string()
                      << "' does not exist!"
                      << std::endl;
            return 0;
        }
        if (boost::filesystem::is_directory(config_file)) {
            std::cout << "The specified config file '"
                      << config_file.string()
                      << "' is a directory!"
                      << std::endl;
            return 0;
        }
        argc -= 2;
        argv += 2;
    }

    if (boost::filesystem::exists(config_file)) {
        const auto& path = config_file.string();
        bc::ifstream file(path);

        if (!file.good()) {
            BOOST_THROW_EXCEPTION(po::reading_file(path.c_str()));
        }

        std::string tmp;
        std::string rpc_version;
        po::options_description desc("");
        desc.add_options()
            ("server.rpc_version", po::value<std::string>(&rpc_version)->default_value(default_rpc_version))
            ("server.mongoose_listen", po::value<std::string>(&tmp)->default_value("127.0.0.1:8820"));

        po::variables_map vm;
        po::store(po::parse_config_file(file, desc, true), vm);
        po::notify(vm);

        if (vm.count("server.mongoose_listen")) {
            if (!tmp.empty()) {
                // On Windows, client can not connect to 0.0.0.0
                if (tmp.find("0.0.0.0") == 0) {
                    tmp.replace(0, 7, "127.0.0.1");
                }
                url = tmp + "/rpc/v" + rpc_version;
            }
        }
    }

    // HTTP request call commands
    HttpReq req(url, 3000, reply_handler(my_impl));

    Json::Value jsonvar;
    jsonvar["jsonrpc"] = "2.0";
    jsonvar["id"] = 1;
    jsonvar["method"] = (argc > 1) ? argv[1] : "help";
    jsonvar["params"] = Json::arrayValue;

    if (argc > 2)
    {
        for (int i = 2; i < argc; i++)
        {
            jsonvar["params"].append(argv[i]);
        }
    }

    req.post(jsonvar.toStyledString());
    return 0;
}
