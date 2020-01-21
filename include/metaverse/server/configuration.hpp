/**
 * Copyright (c) 2011-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2020 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse-server.
 *
 * metaverse-server is free software: you can redistribute it and/or
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
#ifndef MVS_SERVER_CONFIGURATION_HPP
#define MVS_SERVER_CONFIGURATION_HPP

#include <metaverse/node.hpp>
#include <metaverse/server/define.hpp>
#include <metaverse/server/settings.hpp>

namespace libbitcoin {
namespace server {

// Not localizable.
#define BS_HELP_VARIABLE "help"
#define BS_SETTINGS_VARIABLE "settings"
#define BS_VERSION_VARIABLE "version"
#define BS_DAEMON_VARIABLE "daemon"
#define BS_TESTNET_VARIABLE "testnet"
#define BS_DATADIR_VARIABLE "datadir"
#define BS_UI_VARIABLE "ui"


// This must be lower case but the env var part can be any case.
#define BS_CONFIG_VARIABLE "config"

// This must match the case of the env var.
#define BS_ENVIRONMENT_VARIABLE_PREFIX "BS_"

/// Full server node configuration, thread safe.
class BCS_API configuration
  : public node::configuration
{
public:
    configuration(bc::settings context);
    configuration(const configuration& other);

    /// Settings.
    server::settings server;
};

} // namespace server
} // namespace libbitcoin

#endif
