/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS)
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
#include <iostream>
#include <json/minijson_writer.hpp>
#include <metaverse/explorer.hpp>
#include <metaverse/mgbubble/MongooseCli.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

BC_USE_MVS_MAIN

/**
 * Invoke this program with the raw arguments provided on the command line.
 * All console input and output streams for the application originate here.
 * @param argc  The number of elements in the argv array.
 * @param argv  The array of arguments, including the process.
 * @return      The numeric result to return via console exit.
 */
using namespace mgbubble::cli;

void my_impl(const http_message* hm){
    auto&& reply = std::string(hm->body.p, hm->body.len);
    bc::cout<<reply<<std::endl;
}

int bc::main(int argc, char* argv[])
{

    if (argc == 1 || std::memcmp(argv[1], "-h", 2) == 0 ||
                    std::memcmp(argv[1], "--help", 6) == 0)
    {
        bc::explorer::display_usage(bc::cout);
        return console_result::okay;
    }

	// original commands
    std::string cmd{argv[1]};
	auto cmd_ptr = bc::explorer::find_extension(cmd);

    auto is_online_cmd = [](const char* cmd){
        return std::memcmp(cmd, "fetch-", 6) == 0;
    };

	{
		std::ostringstream sout;
		console_result ret;
		try {
			if (!cmd_ptr && !is_online_cmd(cmd.c_str())) {
				ret = bc::explorer::dispatch_command(argc - 1,
					const_cast<const char**>(argv + 1), bc::cin, sout, sout);
				if (ret != console_result::okay) {
					throw explorer::command_params_exception(sout.str());
				}
				std::pair<uint32_t, std::string> ex_pair;
				std::stringstream ex_stream;
				ex_stream.str(sout.str());
				if (explorer::capture_excode(ex_stream, ex_pair) == console_result::okay) {
					throw explorer::explorer_exception(ex_pair.first, ex_pair.second);
				}
			}
		}
		catch (bc::explorer::explorer_exception e) {
			sout.str("");
			sout << e;
		}
		bc::cout << sout.str() << std::endl;
		return ret;
	}


	// extension commands
    HttpReq req("127.0.0.1:8820/rpc", 3000, reply_handler(my_impl));
    std::ostringstream sout{""};
    minijson::object_writer writer(sout);
    writer.write("method", argv[1]);

	if (argc > 2){
        minijson::array_writer awriter = writer.nested_array("params");
        for (int i = 2 ; i < argc; i++) {
            awriter.write(argv[i]);
        }
        awriter.close();
	}

    writer.close();

    req.post(sout.str());

    return 0;
}
