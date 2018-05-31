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
#include <metaverse/explorer/dispatch.hpp>

#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <metaverse/explorer/command.hpp>
#include <metaverse/explorer/extensions/command_extension.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/display.hpp>
#include <metaverse/explorer/generated.hpp>
#include <metaverse/explorer/parser.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/bitcoin.hpp>

using namespace boost::filesystem;
using namespace boost::program_options;
using namespace boost::system;

namespace libbitcoin {
namespace explorer {

// Swap Unicode input stream for binary stream in Windows builds.
static std::istream& get_command_input(command& command, std::istream& input)
{
#ifdef _MSC_VER
    if (command.requires_raw_input())
    {
        bc::set_binary_stdin();
        return std::cin;
    }

    bc::set_utf8_stdin();
#endif

    return input;
}

// Swap Unicode output stream for binary stream in Windows builds.
static std::ostream& get_command_output(command& command, std::ostream& output)
{
#ifdef _MSC_VER
    if (command.requires_raw_output())
    {
        bc::set_binary_stdout();
        return std::cout;
    }

    bc::set_utf8_stdout();
#endif

    return output;
}

// Set Unicode error stream in Windows builds.
static std::ostream& get_command_error(command& command, std::ostream& error)
{
    bc::set_utf8_stderr();
    return error;
}

console_result dispatch(int argc, const char* argv[],
    std::istream& input, std::ostream& output, std::ostream& error)
{
    if (argc == 1)
    {
        display_usage(output);
        return console_result::okay;
    }

    auto ret = dispatch_command(argc - 1, &argv[1], input, output, error);
    output<<std::endl;
    //error<<std::endl; // once \n is okay
    return ret;
}

console_result dispatch_command(int argc, const char* argv[],
    std::istream& input, std::ostream& output, std::ostream& error)
{
    const std::string target(argv[0]);
    const auto command = find(target);

    if (!command)
    {
        const std::string superseding(formerly(target));
        display_invalid_command(error, target, superseding);
        return console_result::failure;
    }

    auto& in = get_command_input(*command, input);
    auto& err = get_command_error(*command, error);
    auto& out = get_command_output(*command, output);

    parser metadata(*command);
    std::string error_message;

    if (!metadata.parse(error_message, in, argc, argv))
    {
        display_invalid_parameter(error, error_message);
        return console_result::failure;
    }

    if (metadata.help())
    {
        command->write_help(output);
        return console_result::okay;
    }

    return command->invoke(out, err);
}

console_result dispatch_command(int argc, const char* argv[],
    Json::Value& jv_output,
    libbitcoin::server::server_node& node, uint8_t api_version)
{
    std::istringstream input;
    std::ostringstream output;

    const std::string target(argv[0]);
    const auto command = find(target);

    if (!command)
    {
        const std::string superseding(formerly(target));
        display_invalid_command(output, target, superseding);
        throw invalid_command_exception{ output.str() };
    }

    auto& in = get_command_input(*command, input);

    parser metadata(*command);
    std::string error_message;

    if (!metadata.parse(error_message, in, argc, argv))
    {
        display_invalid_parameter(output, error_message);
        throw command_params_exception{ output.str() };
    }

    if (metadata.help())
    {
        command->write_help(output);
        jv_output = output.str();
        return console_result::okay;
    }

    command->set_api_version(api_version);

    if (command->category(ctgy_extension))
    {
        // fixme. is_blockchain_sync has some problem.
        // if (command->category(ctgy_online) && node.is_blockchain_sync()) {
        if (command->category(ctgy_online) &&
            !node.chain_impl().chain_settings().use_testnet_rules) {
            uint64_t height{0};
            node.chain_impl().get_last_height(height);
            if (!command->is_block_height_fullfilled(height)) {
                throw block_sync_required_exception{"This command is unavailable because of the height < 610000."};
            }
        }

        return static_cast<commands::command_extension*>(command.get())->invoke(jv_output, node);
    }
    else {
        command->set_api_version(1); // only compatible for v1
        auto retcode = command->invoke(output, output);
        jv_output = output.str();
        return retcode;
    }
}


} // namespace explorer
} // namespace libbitcoin
