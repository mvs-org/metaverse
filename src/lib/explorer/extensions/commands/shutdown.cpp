/**
 * Copyright (c) 2016-2020 mvs developers
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


#include <metaverse/explorer/extensions/commands/shutdown.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/node_method_wrapper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

/************************ shutdown *************************/
console_result shutdown::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();

    administrator_required_checker(node, auth_.name, auth_.auth);

#ifndef _WIN32
    jv_output = "sending SIGTERM to mvsd.";
    killpg(getpgrp(),SIGTERM);
#else
    jv_output = "not support for Windows";
#endif

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

