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
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ getlocked *************************/

class getlocked: public command_extension
{
public:
    static const char* symbol(){ return "getlocked";}
    const char* name() override { return symbol();}
    bool category(int bs) override { return (ex_online & bs ) == bs; }
    const char* description() override { return "Get locked balance of target address."; }

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
            "did/address"
        )
        (
            "expiration,e",
            value<uint64_t>(&option_.expiration)->default_value(0),
            "expiration height, should be still locked at this height."
        )
        (
            "symbol,s",
            value<std::string>(&option_.asset_symbol)->default_value(DEFAULT_INVALID_ASSET_SYMBOL),
            "asset symbol"
        )
        (
            "stake,k",
            value<bool>(&option_.dpos_stake)->default_value(false)->zero_tokens(),
            "If specified, then sum effective locked values for DPoS stake. Defaults to false."
        )
        (
            "range,r",
            value<colon_delimited2_item<uint64_t, uint64_t>>(&option_.range),
            "Pick locked value between this range [begin:end)."
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
        std::string asset_symbol;
        uint64_t expiration;
        colon_delimited2_item<uint64_t, uint64_t> range = {0, 0};
        bool dpos_stake;
    } option_;

};




} // namespace commands
} // namespace explorer
} // namespace libbitcoin

