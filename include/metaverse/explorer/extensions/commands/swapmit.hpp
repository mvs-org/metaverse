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

#define DEFAULT_SWAP_FEE 100000000

/************************ burn *************************/

class swapmit: public command_extension
{
public:
    static const char* symbol() { return "swapmit";}
    const char* name() override { return symbol();}
    bool category(int bs) override { return (ex_online & bs ) == bs; }
    const char* description() override { return "Swap mit for crosschain transaction."; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
               .add("ACCOUNTNAME", 1)
               .add("ACCOUNTAUTH", 1)
               .add("TO_", 1)
               .add("SYMBOL", 1)
               .add("FOREIGN_ADDR", 1);
    }

    void load_fallbacks (std::istream& input,
                         po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "ACCOUNTNAME", variables, input, raw);
        load_input(auth_.auth, "ACCOUNTAUTH", variables, input, raw);
        load_input(argument_.to, "TO_", variables, input, raw);
        load_input(argument_.symbol, "SYMBOL", variables, input, raw);
        load_input(argument_.foreign_addr, "FOREIGN_ADDR", variables, input, raw);
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
            "ACCOUNTNAME",
            value<std::string>(&auth_.name)->required(),
            BX_ACCOUNT_NAME
        )
        (
            "ACCOUNTAUTH",
            value<std::string>(&auth_.auth)->required(),
            BX_ACCOUNT_AUTH
        )
        (
            "TO_",
            value<std::string>(&argument_.to)->required(),
            "To this did/address the specific mit will be sent. expect to be \"droplet\"."
        )
        (
            "SYMBOL",
            value<std::string>(&argument_.symbol)->required(),
            "Asset symbol"
        )
        (
            "FOREIGN_ADDR",
            value<std::string>(&argument_.foreign_addr)->required(),
            "To this address of the destination chain to swap the mit."
        )
        (
            "memo,m",
            value<std::string>(&option_.memo)->default_value(""),
            "Attached memo for this transaction."
        )
        (
            "swapfee,s",
            value<uint64_t>(&option_.swapfee)->default_value(DEFAULT_SWAP_FEE),
            "Transaction fee for crosschain token swap. defaults to 1 ETP"
        )
        (
            "fee,f",
            value<uint64_t>(&option_.fee)->default_value(10000),
            "Transaction fee for miners. defaults to 10000 ETP bits"
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
        std::string to;
        std::string symbol;
        std::string foreign_addr;
    } argument_;

    struct option
    {
        uint64_t swapfee;
        uint64_t fee;
        std::string memo;
    } option_;

};

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

