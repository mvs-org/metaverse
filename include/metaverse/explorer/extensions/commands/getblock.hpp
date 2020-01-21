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


/************************ getblock *************************/

class getblock: public command_extension
{
public:
    static const char* symbol(){ return "getblock";}
    const char* name() override { return symbol();}
    bool category(int bs) override { return (ctgy_extension & bs ) == bs; }
    const char* description() override { return "Get sepcified block header from wallet."; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
                .add("HASH_OR_HEIGH", 1)
                .add("json", 1)
                .add("tx_json", 1);
    }

    void load_fallbacks (std::istream& input,
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(argument_.hash_or_height, "HASH_OR_HEIGH", variables, input, raw);
        load_input(argument_.hash_or_height, "json", variables, input, raw);
        load_input(argument_.hash_or_height, "tx_json", variables, input, raw);
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
            "HASH_OR_HEIGH",
            value<std::string>(&argument_.hash_or_height)->required(),
            "block hash or block height"
        )
        (
            "json",
            value<bool>(&option_.json)->default_value(true),
            "Json/Raw format, default is '--json=true'."
        )
        (
            "tx_json",
            value<bool>(&option_.tx_json)->default_value(true),
            "Json/Raw format for txs, default is '--tx_json=true'."
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
        std::string hash_or_height;
    } argument_;

    struct option
    {
        bool json;
        bool tx_json;
    } option_;

};


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

