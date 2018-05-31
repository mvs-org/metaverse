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


/************************ createrawtx *************************/

class createrawtx: public command_extension
{
public:
    static const char* symbol(){ return "createrawtx";}
    const char* name() override { return symbol();}
    bool category(int bs) override { return (ctgy_extension & bs ) == bs; }
    const char* description() override { return "createrawtx"; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata();
    }

    void load_fallbacks (std::istream& input,
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
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
            "type,t",
            value<uint16_t>(&option_.type)->required(),
            "Transaction type. 0 -- transfer etp, 1 -- deposit etp, 3 -- transfer asset"
        )
        (
            "senders,s",
            value<std::vector<std::string>>(&option_.senders)->required(),
            "Send from addresses"
        )
        (
            "receivers,r",
            value<std::vector<std::string>>(&option_.receivers)->required(),
            "Send to [address:amount]. amount is asset number if sybol option specified"
        )
        (
            "symbol,n",
            value<std::string>(&option_.symbol)->default_value(""),
            "asset name, not specify this option for etp tx"
        )
        (
            "deposit,d",
            value<uint16_t>(&option_.deposit)->default_value(7),
            "Deposits support [7, 30, 90, 182, 365] days. defaluts to 7 days"
        )
        (
            "mychange,m",
            value<std::string>(&option_.mychange_address),
            "Mychange to this address, includes etp and asset change"
        )
        (
            "message,i",
            value<std::string>(&option_.message),
            "Message/Information attached to this transaction"
        )
        (
            "fee,f",
            value<uint64_t>(&option_.fee)->default_value(10000),
            "Transaction fee. defaults to 10000 ETP bits"
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
        argument()
        {
        }
    } argument_;

    struct option
    {
        uint16_t type;
        std::vector<std::string> senders;
        std::vector<std::string> receivers;
        std::string symbol;
        std::string mychange_address;
        std::string message;
        uint16_t deposit;
        uint64_t fee;

    } option_;

};

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

