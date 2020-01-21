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


#pragma once
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/extensions/command_extension.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ getaddressasset *************************/

class getaddressasset: public command_extension
{
public:
    static const char* symbol(){ return "getaddressasset";}
    const char* name() override { return symbol();}
    bool category(int bs) override { return (ex_online & bs ) == bs; }
    const char* description() override { return "getaddressasset "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ADDRESS", 1);
    }

    void load_fallbacks (std::istream& input,
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(argument_.address, "ADDRESS", variables, input, raw);
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
            "ADDRESS",
            value<std::string>(&argument_.address)->required(),
            "address"
        )
        (
            "cert,c",
            value<bool>(&option_.is_cert)->default_value(false)->zero_tokens(),
            "If specified, then only get related cert. Default is not specified."
        )
        (
            "deposited,d",
            value<bool>(&option_.deposited)->zero_tokens()->default_value(false),
            "If specified, then only get deposited assets. Default is not specified."
        )
        (
            "symbol,s",
            value<std::string>(&option_.symbol)->default_value(""),
            "Asset symbol."
        )
        ;

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
        option():
            is_cert(false),
            deposited(false)
        {}

        bool is_cert;
        bool deposited;
        std::string symbol;
    } option_;

};




} // namespace commands
} // namespace explorer
} // namespace libbitcoin

