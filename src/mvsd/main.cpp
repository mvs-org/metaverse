/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin-server.
 *
 * libbitcoin-server is free software: you can redistribute it and/or
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
#include <bitcoin/server.hpp>
#include <bitcoin/bitcoin/utility/backtrace.hpp>
#include "executor.hpp"

BC_USE_MVS_MAIN

/**
 * Invoke this program with the raw arguments provided on the command line.
 * All console input and output streams for the application originate here.
 * @param argc  The number of elements in the argv array.
 * @param argv  The array of arguments, including the process.
 * @return      The numeric result to return via console exit.
 */
int bc::main(int argc, char* argv[])
{
    using namespace bc;
    using namespace bc::server;

    try{
		set_utf8_stdio();
		server::parser metadata(bc::settings::mainnet);
		const auto& args = const_cast<const char**>(argv);

		if (!metadata.parse(argc, args, cerr))
			return console_result::failure;

		executor host(metadata, cin, cout, cerr);
		return host.menu() ? console_result::okay : console_result::failure;
    }
	catch(const std::exception& e)
	{
		do_backtrace("exception.out");
	}
	catch(...)
	{
		do_backtrace("exception.out");
	}
    return console_result::failure;

}
