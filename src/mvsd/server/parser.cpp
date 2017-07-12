/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS)
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
#include <metaverse/server/parser.hpp>

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <metaverse/node.hpp>
#include <metaverse/server/define.hpp>
#include <metaverse/server/parser.hpp>
#include <metaverse/server/settings.hpp>
#include <metaverse/bitcoin/utility/path.hpp>

BC_DECLARE_CONFIG_DEFAULT_PATH(".metaverse" / "mvs.conf")

// TODO: localize descriptions.

namespace libbitcoin {
namespace server {

using namespace boost::filesystem;
using namespace boost::program_options;
using namespace bc::config;
using namespace bc::network;

// Initialize configuration by copying the given instance.
parser::parser(const configuration defaults)
  : configured(defaults)
{
}

// Initialize configuration using defaults of the given context.
parser::parser(bc::settings context)
  : configured(context)
{
}

options_metadata parser::load_options()
{
    options_metadata description("options");
    description.add_options()
    (
        BS_CONFIG_VARIABLE ",c",
        value<path>(&configured.file)->
		default_value("mvs.conf"),
        "Specify path to a configuration settings file based on path ~/.metaverse"
    )
    (
        BS_HELP_VARIABLE ",h",
        value<bool>(&configured.help)->
            default_value(false)->zero_tokens(),
        "Display command line options."
    )
    (
        "initchain,i",
        value<bool>(&configured.initchain)->
            default_value(false)->zero_tokens(),
        "Initialize blockchain in the configured directory."
    )
    (
        BS_SETTINGS_VARIABLE ",s",
        value<bool>(&configured.settings)->
            default_value(false)->zero_tokens(),
        "Display all configuration settings."
    )
    (
        BS_VERSION_VARIABLE ",v",
        value<bool>(&configured.version)->
            default_value(false)->zero_tokens(),
        "Display version information."
    )
	(
		BS_DAEMON_VARIABLE ",d",
		value<bool>(&configured.daemon)->
			default_value(false)->zero_tokens(),
		"Run in daemon mode (unix/apple)."
	)
	(
		"testnet,t",
		value<bool>(&configured.use_testnet_rules)->
			default_value(false)->zero_tokens(),
		"Use testnet rules for determination of work required, defaults to false."
	)
    (
        BS_DATADIR_VARIABLE ",D",
        value<path>(&configured.data_dir)->
            default_value(""),
        "Specify database path."
    )
	;

    return description;
}

arguments_metadata parser::load_arguments()
{
    arguments_metadata description;
    return description
        .add(BS_CONFIG_VARIABLE, 1);
}

options_metadata parser::load_environment()
{
    options_metadata description("environment");
    description.add_options()
    (
        // For some reason po requires this to be a lower case name.
        // The case must match the other declarations for it to compose.
        // This composes with the cmdline options and inits to system path.
        BS_CONFIG_VARIABLE,
        value<path>(&configured.file)->composing()
            ->default_value(config_default_path()),
        "The path to the configuration settings file."
    );

    return description;
}

options_metadata parser::load_settings()
{
    options_metadata description("settings");
    description.add_options()
    /* [network] */
    (
        "network.threads",
        value<uint32_t>(&configured.network.threads),
        "The minimum number of threads in the application threadpool, defaults to 50."
    )
    (
        "network.protocol",
        value<uint32_t>(&configured.network.protocol),
        "The network protocol version, defaults to 70012."
    )
    (
        "network.identifier",
        value<uint32_t>(&configured.network.identifier),
        "The magic number for message headers, defaults to 0x6d73766d."
    )
    (
        "network.inbound_port",
        value<uint16_t>(&configured.network.inbound_port),
        "The port for incoming connections, defaults to 5251."
    )
    (
        "network.inbound_connections",
        value<uint32_t>(&configured.network.inbound_connections),
        "The target number of incoming network connections, defaults to 8."
    )
    (
        "network.outbound_connections",
        value<uint32_t>(&configured.network.outbound_connections),
        "The target number of outgoing network connections, defaults to 8."
    )
    (
        "network.manual_attempt_limit",
        value<uint32_t>(&configured.network.manual_attempt_limit),
        "The attempt limit for manual connection establishment, defaults to 0 (forever)."
    )
    (
        "network.connect_batch_size",
        value<uint32_t>(&configured.network.connect_batch_size),
        "The number of concurrent attempts to estalish one connection, defaults to 5."
    )
    (
        "network.connect_timeout_seconds",
        value<uint32_t>(&configured.network.connect_timeout_seconds),
        "The time limit for connection establishment, defaults to 5."
    )
    (
        "network.channel_handshake_seconds",
        value<uint32_t>(&configured.network.channel_handshake_seconds),
        "The time limit to complete the connection handshake, defaults to 30."
    )
    (
        "network.channel_heartbeat_minutes",
        value<uint32_t>(&configured.network.channel_heartbeat_minutes),
        "The time between ping messages, defaults to 5."
    )
    (
        "network.channel_inactivity_minutes",
        value<uint32_t>(&configured.network.channel_inactivity_minutes),
        "The inactivity time limit for any connection, defaults to 30."
    )
    (
        "network.channel_expiration_minutes",
        value<uint32_t>(&configured.network.channel_expiration_minutes),
        "The maximum age limit for an outbound connection, defaults to 1440."
    )
    (
        "network.channel_germination_seconds",
        value<uint32_t>(&configured.network.channel_germination_seconds),
        "The maximum time limit for obtaining seed addresses, defaults to 30."
    )
    (
        "network.host_pool_capacity",
        value<uint32_t>(&configured.network.host_pool_capacity),
        "The maximum number of peer hosts in the pool, defaults to 1000."
    )
    (
        "network.relay_transactions",
        value<bool>(&configured.network.relay_transactions),
        "Request that peers relay transactions, defaults to true."
    )
    (
        "network.hosts_file",
        value<path>(&configured.network.hosts_file),
        "The peer hosts cache file path, defaults to 'hosts.cache'."
    )
    (
        "network.debug_file",
        value<path>(&configured.network.debug_file),
        "The debug log file path, defaults to 'debug.log'."
    )
    (
        "network.error_file",
        value<path>(&configured.network.error_file),
        "The error log file path, defaults to 'error.log'."
    )
    (
        "network.self",
        value<config::authority>(&configured.network.self),
        "The advertised public address of this node, defaults to none."
    )
    (
        "network.blacklist",
        value<config::authority::list>(&configured.network.blacklists),
        "IP address to disallow as a peer, multiple entries allowed."
    )
    (
        "network.peer",
        value<config::endpoint::list>(&configured.network.peers),
        "Persistent host:port channels, multiple entries allowed."
    )
    (
        "network.seed",
        value<config::endpoint::list>(&configured.network.seeds),
        "A seed node for initializing the host pool, multiple entries allowed."
    )

    /* [database] */
    (
        "database.history_start_height",
        value<uint32_t>(&configured.database.history_start_height),
        "The lower limit of spend indexing, defaults to 0."
    )
    (
        "database.stealth_start_height",
        value<uint32_t>(&configured.database.stealth_start_height),
        "The lower limit of stealth indexing, defaults to 350000."
    )
    (
        "database.directory",
        value<path>(&configured.database.directory),
        "The blockchain database directory, defaults to 'mainnet'."
    )

    /* [blockchain] */
    (
        "blockchain.block_pool_capacity",
        value<uint32_t>(&configured.chain.block_pool_capacity),
        "The maximum number of orphan blocks in the pool, defaults to 50."
    )
    (
        "blockchain.transaction_pool_capacity",
        value<uint32_t>(&configured.chain.transaction_pool_capacity),
        "The maximum number of transactions in the pool, defaults to 2000."
    )
    (
        "blockchain.transaction_pool_consistency",
        value<bool>(&configured.chain.transaction_pool_consistency),
        "Enforce consistency between the pool and the blockchain, defaults to false."
    )
    (
        "blockchain.use_testnet_rules",
        value<bool>(&configured.chain.use_testnet_rules),
        "Use testnet rules for determination of work required, defaults to false."
    )
    (
        "blockchain.checkpoint",
        value<config::checkpoint::list>(&configured.chain.checkpoints),
        "A hash:height checkpoint, multiple entries allowed."
    )

    /* [node] */
    (
        "node.block_timeout_seconds",
        value<uint32_t>(&configured.node.block_timeout_seconds),
        "The time limit for block receipt during initial block download, defaults to 5."
    )
    (
        "node.download_connections",
        value<uint32_t>(&configured.node.download_connections),
        "The maximum number of connections for initial block download, defaults to 8."
    )
    (
        "node.transaction_pool_refresh",
        value<bool>(&configured.node.transaction_pool_refresh),
        "Refresh the transaction pool on reorganization and channel start, defaults to true."
    )

    /* [server] */
	(
		"server.mongoose_listen",
		value<std::string>(&configured.server.mongoose_listen),
		"the listening port for mongoose, defaults to 127.0.0.1:8820."
	)
    (
        "server.query_workers",
        value<uint16_t>(&configured.server.query_workers),
        "The number of query worker threads per endpoint, defaults to 1."
    )
    (
        "server.heartbeat_interval_seconds",
        value<uint32_t>(&configured.server.heartbeat_interval_seconds),
        "The heartbeat interval, defaults to 5."
    )
    (
        "server.subscription_expiration_minutes",
        value<uint32_t>(&configured.server.subscription_expiration_minutes),
        "The subscription expiration time, defaults to 10."
    )
    (
        "server.subscription_limit",
        value<uint32_t>(&configured.server.subscription_limit),
        "The maximum number of subscriptions, defaults to 100000000."
    )
    (
        "server.log_requests",
        value<bool>(&configured.server.log_requests),
        "Write service requests to the log, defaults to false."
    )
    (
        "server.secure_only",
        value<bool>(&configured.server.secure_only),
        "Disable public endpoints, defaults to false."
    )
    (
        "server.query_service_enabled",
        value<bool>(&configured.server.query_service_enabled),
        "Enable the query service, defaults to true."
    )
    (
        "server.heartbeat_service_enabled",
        value<bool>(&configured.server.heartbeat_service_enabled),
        "Enable the heartbeat service, defaults to false."
    )
    (
        "server.block_service_enabled",
        value<bool>(&configured.server.block_service_enabled),
        "Enable the block publishing service, defaults to false."
    )
    (
        "server.transaction_service_enabled",
        value<bool>(&configured.server.transaction_service_enabled),
        "Enable the transaction publishing service, defaults to false."
    )
    (
        "server.public_query_endpoint",
        value<endpoint>(&configured.server.public_query_endpoint),
        "The public query endpoint, defaults to 'tcp://*:9091'."
    )
    (
        "server.public_heartbeat_endpoint",
        value<endpoint>(&configured.server.public_heartbeat_endpoint),
        "The public heartbeat endpoint, defaults to 'tcp://*:9092'."
    )
    (
        "server.public_block_endpoint",
        value<endpoint>(&configured.server.public_block_endpoint),
        "The public block publishing endpoint, defaults to 'tcp://*:9093'."
    )
    (
        "server.public_transaction_endpoint",
        value<endpoint>(&configured.server.public_transaction_endpoint),
        "The public transaction publishing endpoint, defaults to 'tcp://*:9094'."
    )
    (
        "server.secure_query_endpoint",
        value<endpoint>(&configured.server.secure_query_endpoint),
        "The secure query endpoint, defaults to 'tcp://*:9081'."
    )
    (
        "server.secure_heartbeat_endpoint",
        value<endpoint>(&configured.server.secure_heartbeat_endpoint),
        "The secure heartbeat endpoint, defaults to 'tcp://*:9082'."
    )
    (
        "server.secure_block_endpoint",
        value<endpoint>(&configured.server.secure_block_endpoint),
        "The secure block publishing endpoint, defaults to 'tcp://*:9083'."
    )
    (
        "server.secure_transaction_endpoint",
        value<endpoint>(&configured.server.secure_transaction_endpoint),
        "The secure transaction publishing endpoint, defaults to 'tcp://*:9084'."
    )
    (
        "server.server_private_key",
        value<config::sodium>(&configured.server.server_private_key),
        "The Z85-encoded private key of the server, enables secure endpoints."
    )
    (
        "server.client_public_key",
        value<config::sodium::list>(&configured.server.client_public_keys),
        "Allowed Z85-encoded public key of the client, multiple entries allowed."
    )
    (
        "server.client_address",
        value<config::authority::list>(&configured.server.client_addresses),
        "Allowed client IP address, multiple entries allowed."
    );

    return description;
}

bool parser::parse(int argc, const char* argv[], std::ostream& error)
{
    try
    {
        auto file = false;
        variables_map variables;
        load_command_variables(variables, argc, argv);
        load_environment_variables(variables, BS_ENVIRONMENT_VARIABLE_PREFIX);

        // Don't load the rest if any of these options are specified.
        if (!get_option(variables, BS_VERSION_VARIABLE) && 
            !get_option(variables, BS_SETTINGS_VARIABLE) &&
            !get_option(variables, BS_HELP_VARIABLE))
        {
        	if (get_option(variables, BS_TESTNET_VARIABLE))
			{
				configured.network.hosts_file = "hosts-test.cache";
				const_cast<path&>(variables[BS_CONFIG_VARIABLE].as<path>()) = "mvs-test.conf";
			}
            auto data_dir = variables[BS_DATADIR_VARIABLE].as<path>();
            if (!data_dir.empty())
            {
                if (boost::filesystem::exists(data_dir) && !boost::filesystem::is_directory(data_dir))
                {
                    error << format_invalid_parameter("datadir path is invalid.") << std::endl;
                    return false;
                } else {
                    set_default_data_path(data_dir);
                }
            }
            // Returns true if the settings were loaded from a file.
            file = load_configuration_variables(variables, BS_CONFIG_VARIABLE);
        }

        // Update bound variables in metadata.settings.
        notify(variables);

        // Clear the config file path if it wasn't used.
        if (!file)
            configured.file.clear();
    }
    catch (const boost::program_options::error& e)
    {
        // This is obtained from boost, which circumvents our localization.
        error << format_invalid_parameter(e.what()) << std::endl;
        return false;
    }

    return true;
}

} // namespace server
} // namespace libbitcoin
