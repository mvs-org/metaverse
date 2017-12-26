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
#ifndef BX_DISPATCH_HPP
#define BX_DISPATCH_HPP

#include <iostream>
#include <metaverse/bitcoin.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/server/server_node.hpp>

/* NOTE: don't declare 'using namespace foo' in headers. */

namespace libbitcoin {
namespace explorer {
   
/**
 * Dispatch the command with the raw arguments as provided on the command line.
 * @param[in]  argc    The number of elements in the argv array.
 * @param[in]  argv    The array of arguments, including the process.
 * @param[in]  input   The input stream (e.g. STDIO).
 * @param[in]  output  The output stream (e.g. STDOUT).
 * @param[in]  error   The error stream (e.g. STDERR).
 * @return             The appropriate console return code { -1, 0, 1 }.
 */
BCX_API console_result dispatch(int argc, const char* argv[],
    std::istream& input, std::ostream& output, std::ostream& error);

/**
 * Invoke the command identified by the specified arguments.
 * The first argument in the array is the command symbolic name.
 * @param[in]  argc   The number of elements in the argv parameter.
 * @param[in]  argv   Array of command line arguments excluding the process.
 * @param[in]  input   The input stream (e.g. STDIO).
 * @param[in]  output  The output stream (e.g. STDOUT).
 * @param[in]  error   The error stream (e.g. STDERR).
 * @return            The appropriate console return code { -1, 0, 1 }.
 */
BCX_API console_result dispatch_command(int argc, const char* argv[],
    std::istream& input, std::ostream& output, std::ostream& error);

/**
 * Invoke the command identified by the specified arguments.
 * The first argument in the array is the command symbolic name.
 * @param[in]  argc   The number of elements in the argv parameter.
 * @param[in]  argv   Array of command line arguments excluding the process.
 * @param[in]  node server_node instance.
 * @param[in]  command version, defaults to v1.
 * @return            The appropriate console return code { -1, 0, 1 }.
 */
BCX_API console_result dispatch_command(int argc, const char* argv[],
    Json::Value& jv_output, 
    bc::server::server_node& node, uint8_t api_version = 1);

BCX_API console_result dispatch_command(int argc, const char* argv[],
        std::ostream& cmd_output, Json::Value& jv_output, 
    bc::server::server_node& node, uint8_t api_version = 1);

} // namespace explorer
} // namespace libbitcoin

#endif
