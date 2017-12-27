/**
 * Copyright (c) 2016-2017 mvs developers 
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


#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/extensions/command_extension.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ listtxs *************************/

class listtxs: public command_extension
{
public:
    static const char* symbol(){ return "listtxs";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    bool category(int bs) override { return (ex_online & bs ) == bs; }
    const char* description() override { return "List transactions details of this account."; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ACCOUNTNAME", 1)
            .add("ACCOUNTAUTH", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "ACCOUNTNAME", variables, input, raw);
        load_input(auth_.auth, "ACCOUNTAUTH", variables, input, raw);
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
            "address,a",
            value<std::string>(&argument_.address),
            "Address."
	    )
	    (
            "height,e",
            value<libbitcoin::explorer::commands::colon_delimited2_item<uint64_t, uint64_t>>(&option_.height),
            "Get tx according height eg: -e start-height:end-height will return tx between [start-height, end-height)"
        )
        (
            "symbol,s",
            value<std::string>(&argument_.symbol),
            "Asset symbol."
        )
        (
            "limit,l",
            value<uint64_t>(&argument_.limit)->default_value(100),
            "Transaction count per page."
        )
        (
            "index,i",
            value<uint64_t>(&argument_.index)->default_value(1),
            "Page index."
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
    	argument():address(""), symbol(""), limit(100), index(0)
		{};
    	std::string address;
		std::string symbol;
        uint64_t limit;
        uint64_t index;
    } argument_;

    struct option
    {
    	option():height(0, 0)
		{};
    	libbitcoin::explorer::commands::colon_delimited2_item<uint64_t, uint64_t> height;
    } option_;

};




} // namespace commands
} // namespace explorer
} // namespace libbitcoin

