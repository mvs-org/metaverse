/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
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

#include <metaverse/explorer/display.hpp>

#include <iostream>
#include <memory>
#include <boost/format.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/generated.hpp>
#include <metaverse/explorer/utility.hpp>
#include <metaverse/explorer/version.hpp>

namespace libbitcoin {
namespace explorer {

using namespace bc::config;

void display_command_names(std::ostream& stream)
{
    const auto func = [&stream](std::shared_ptr<command> explorer_command)
    {
        BITCOIN_ASSERT(explorer_command != nullptr);
        if (!explorer_command->obsolete())
            stream << "  " <<explorer_command->name() << "\r\n";
    };

    broadcast(func, stream);
}

void display_connection_failure(std::ostream& stream, const endpoint& url)
{
    stream << format(BX_CONNECTION_FAILURE) % url ;
}

void display_invalid_command(std::ostream& stream, const std::string& command,
    const std::string& superseding)
{
    if (superseding.empty())
        stream << format(BX_INVALID_COMMAND) % command;
    else
        stream << format(BX_DEPRECATED_COMMAND) % command % superseding;
}

// English only hack to patch missing arg name in boost exception message.
static std::string fixup_boost_po_what_en(const std::string& what)
{
    std::string message(what);
    boost::replace_all(message, "for option is invalid", "is invalid");
    return message;
}

void display_invalid_parameter(std::ostream& stream,
    const std::string& message)
{
    stream << format(BX_INVALID_PARAMETER) % fixup_boost_po_what_en(message);
}

void display_usage(std::ostream& stream)
{
    stream
        << std::endl << BX_COMMAND_USAGE << std::endl
        << format(BX_VERSION_MESSAGE) %
            MVS_EXPLORER_VERSION << std::endl
        << BX_COMMANDS_HEADER << std::endl
        << std::endl;

    display_command_names(stream);
}

} // namespace explorer
} // namespace libbitcoin
