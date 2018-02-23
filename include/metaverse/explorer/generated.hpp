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
#ifndef BX_GENERATED_HPP
#define BX_GENERATED_HPP

#include <functional>
#include <memory>
#include <string>
#include <metaverse/explorer/command.hpp>
#include <metaverse/explorer/commands/fetch-history.hpp>
#include <metaverse/explorer/commands/fetch-stealth.hpp>
#include <metaverse/explorer/commands/help.hpp>
#include <metaverse/explorer/commands/send-tx.hpp>
#include <metaverse/explorer/commands/settings.hpp>
#include <metaverse/explorer/commands/stealth-decode.hpp>
#include <metaverse/explorer/commands/stealth-encode.hpp>
#include <metaverse/explorer/commands/stealth-public.hpp>
#include <metaverse/explorer/commands/stealth-secret.hpp>
#include <metaverse/explorer/commands/stealth-shared.hpp>
#include <metaverse/explorer/commands/tx-decode.hpp>
#include <metaverse/explorer/commands/validate-tx.hpp>
#include <metaverse/explorer/define.hpp>

/********* GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY **********/

namespace libbitcoin {
namespace explorer {

/**
 * Various shared localizable strings.
 */
#define BX_COMMANDS_HEADER                                            \
    "Info: The commands are:"
#define BX_COMMANDS_HOME_PAGE                                         \
    "MVS home page:"
#define BX_COMMAND_USAGE                                              \
    "Usage: help COMMAND"
#define BX_CONFIG_DESCRIPTION                                         \
    "The path to the configuration settings file."
#define BX_CONNECTION_FAILURE                                         \
    "Could not connect to server: %1%"
#define BX_DEPRECATED_COMMAND                                         \
    "The '%1%' command has been replaced by '%2%'."
#define BX_HELP_DESCRIPTION                                           \
    "Get a description and instructions for this command."
#define BX_INVALID_COMMAND                                            \
    "'%1%' is not a command. Enter 'help' for a list of commands."
#define BX_INVALID_PARAMETER                                          \
    "%1%"
#define BX_PRINTER_ARGUMENT_TABLE_HEADER                              \
    "Arguments (positional):"
#define BX_PRINTER_DESCRIPTION_FORMAT                                 \
    "Info: %1%"
#define BX_PRINTER_OPTION_TABLE_HEADER                                \
    "Options (named):"
#define BX_PRINTER_USAGE_FORMAT                                       \
    "Usage: %1% %2% %3%"
#define BX_PRINTER_VALUE_TEXT                                         \
    "VALUE"
#define BX_VERSION_MESSAGE                                            \
    "Version: %1%"

/**
 * Invoke a specified function on all commands.
 * @param[in]  func  The function to invoke on all commands.
 */
void broadcast(const std::function<void(std::shared_ptr<command>)> func, std::ostream& os);

/**
 * Find the command identified by the specified symbolic command name.
 * @param[in]  symbol  The symbolic command name.
 * @return             An instance of the command or nullptr if not found.
 */
std::shared_ptr<command> find(const std::string& symbol);

/**
 * Find the new name of the formerly-named command.
 * @param[in]  former  The former symbolic command name.
 * @return             The current name of the formerly-named command.
 */
std::string formerly(const std::string& former);

} // namespace explorer
} // namespace libbitcoin

#endif
