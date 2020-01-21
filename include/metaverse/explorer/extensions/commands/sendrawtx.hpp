/**
 * Copyright (c) 2019-2020 mvs developers
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


/************************ sendrawtx *************************/

class sendrawtx: public command_extension
{
public:
    static const char* symbol() { return "sendrawtx";}
    const char* name() override { return symbol();}
    bool category(int bs) override { return (ctgy_extension & bs ) == bs; }
    const char* description() override { return "sendrawtx "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
               .add("TRANSACTION", 1);
    }

    void load_fallbacks (std::istream& input,
                         po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(argument_.transaction, "TRANSACTION", variables, input, raw);
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
            "TRANSACTION",
            value<explorer::config::transaction>(&argument_.transaction)->required(),
            "The input Base16 transaction to broadcast."
        )
        (
            "fee,f",
            value<uint64_t>(&argument_.fee)->default_value(1000000000),
            "The max tx fee. default_value 10 etp"
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
        explorer::config::transaction transaction;
        uint64_t                      fee;
    } argument_;

    struct option
    {
    } option_;

};

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

