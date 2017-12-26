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


/************************ gettx *************************/

class gettx: public command_extension
{
public:
    static const char* symbol(){ return "gettx";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "gettx "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("HASH", 1)
            .add("json", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(argument_.hash, "HASH", variables, input, raw);
        load_input(argument_.hash, "json", variables, input, raw);
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
            "json",
            value<bool>(&option_.json)->default_value(true),
            "Json/Raw format, default is '--json=true'."
        )
	    (
            "HASH",
            value<bc::config::hash256>(&argument_.hash)->required(),
            "The Base16 transaction hash of the transaction to get. If not specified the transaction hash is read from STDIN."
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
        bc::config::hash256 hash;
    } argument_;

    struct option
    {
        bool json;
    } option_;

};


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

