/**
 * Copyright (c) 2016-2018 mvs developers
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

#pragma once
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/extensions/command_extension.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ addnode *************************/

class addnode: public command_extension
{
public:
    static const char* symbol(){ return "addnode";}
    const char* name() override { return symbol();}
    bool category(int bs) override { return (ctgy_extension & bs ) == bs; }
    const char* description() override { return "This command is used to add/remove p2p node."; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("NODEADDRESS", 1)
            .add("ADMINNAME", 1)
            .add("ADMINAUTH", 1);
    }

    void load_fallbacks (std::istream& input,
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(argument_.address, "NODEADDRESS", variables, input, raw);
        load_input(auth_.name, "ADMINNAME", variables, input, raw);
        load_input(auth_.auth, "ADMINAUTH", variables, input, raw);
    }

    options_metadata& load_options() override
    {
        using namespace po;
        options_description& options = get_option_metadata();
        options.add_options()
        (
            BX_HELP_VARIABLE ",h",
            value<bool>()->zero_tokens(),
            "Get a description and instructions for this command."
        )
        (
            "NODEADDRESS",
            value<std::string>(&argument_.address),
            "The target node address[x.x.x.x:port], required by all operations except reseed."
        )
        (
            "ADMINNAME",
            value<std::string>(&auth_.name),
            "admin name."
        )
        (
            "ADMINAUTH",
            value<std::string>(&auth_.auth),
            "admin password/authorization."
        )
        (
            "operation,o",
            value<std::string>(&option_.operation),
            "The operation[ add|ban|peer|reseed ] to the target node address. default: add."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node) override;

    struct argument
    {
        std::string address;
    } argument_;

    struct option
    {
        std::string operation;
    } option_;

};




} // namespace commands
} // namespace explorer
} // namespace libbitcoin

