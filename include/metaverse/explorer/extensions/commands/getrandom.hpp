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


/************************ getrandom *************************/

class getrandom: public command_extension
{
public:
    static const char* symbol(){ return "getrandom";}
    const char* name() override { return symbol();}
    bool category(int bs) override { return (ctgy_extension & bs ) == bs; }
    const char* description() override {
        return "get a random integer between specified range. "
                "For the argument POINT1 and POINT2, "
                "If both are not specified, the range is [0, max_uint64]. "
                "If only POINT1 is specified, the range is [0, POINT1]. "
                "If both are specified, the range is [POINT1, POINT2] (or [POINT2, POINT1] if POINT2 < POINT1).";
    }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("POINT1", 1)
            .add("POINT2", 1);
    }

    void load_fallbacks (std::istream& input,
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(argument_.point1, "POINT1", variables, input, raw);
        load_input(argument_.point2, "POINT2", variables, input, raw);
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
            "POINT1",
            value<uint64_t>(&argument_.point1)->default_value(0),
            "One point of the range."
        )
        (
            "POINT2",
            value<uint64_t>(&argument_.point2)->default_value(max_uint64),
            "Another point of the range."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
        const auto& var_point1 = variables["POINT1"];
        const auto& var_point2 = variables["POINT2"];
        if (!var_point1.defaulted() && var_point2.defaulted())
        {
            argument_.point2 = 0;
        }
    }

    console_result invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node) override;

    struct argument
    {
        uint64_t point1;
        uint64_t point2;
    } argument_;

    struct option
    {
    } option_;
};




} // namespace commands
} // namespace explorer
} // namespace libbitcoin

