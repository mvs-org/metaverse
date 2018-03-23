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


#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/extensions/command_extension.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ createasset *************************/
struct non_negative_uint64
{
public:
    uint64_t volume;
};

void validate(boost::any& v,
			  const std::vector<std::string>& values,
			  non_negative_uint64*, int);

class createasset: public command_extension
{
public:
    static const char* symbol(){ return "createasset";}
    const char* name() override { return symbol();} 
    bool category(int bs) override { return (ctgy_extension & bs ) == bs; }
    const char* description() override { return "createasset "; }

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
	        "rate,r",
		    value<int32_t>(&option_.secondaryissue_threshold),
		    "The rate of secondaryissue. Default to 0, means the asset is not allowed to secondary issue forever; otherwise, -1 means the asset can be secondary issue freely; otherwise, the valid rate is in range of 1 to 100, means the asset can be secondary issue when own percentage greater than the rate value."
		)
	    (
            "symbol,s",
            value<std::string>(&option_.symbol)->required(),
            "The asset symbol/name. Global unique."
        )
        (
            "volume,v",
            value<non_negative_uint64>(&option_.maximum_supply)->required(),
            "The asset maximum supply volume."
        )
        (
            "decimalnumber,n",
            value<uint32_t>(&option_.decimal_number),
            "The asset amount decimal number, defaults to 0."
        )
		(
            "issuer,i",
            value<std::string>(&option_.issuer),
            "The asset issuer.defaults to account name."
        )
        (
            "description,d",
            value<std::string>(&option_.description),
            "The asset description, defaults to empty string."
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
    } argument_;

    struct option
    {
    	option()
		  : symbol(""),
			maximum_supply{0},
			decimal_number(0),
            secondaryissue_threshold(0),
			issuer(""),
			description("")
    	{
    	};
		
		std::string symbol;
		non_negative_uint64 maximum_supply;
		uint32_t decimal_number;
		int32_t secondaryissue_threshold;
		std::string issuer; 
		std::string description;
    } option_;

};


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

