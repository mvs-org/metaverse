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


class importaccount: public command_extension
{
public:
    static const char* symbol(){ return "importaccount";}
    const char* name() override { return symbol();} 
    bool category(int bs) override { return (ctgy_extension & bs ) == bs; }
    const char* description() override { return "importaccount "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("WORD", -1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(argument_.words, "WORD", variables, input, raw);
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
			"WORD",
			value<std::vector<std::string>>(&argument_.words)->required(),
			"The set of words that that make up the mnemonic. If not specified the words are read from STDIN."
		)
		(
			"language,l",
			value<explorer::config::language>(&option_.language),
			"The language identifier of the dictionary of the mnemonic. Options are 'en', 'es', 'ja', 'zh_Hans', 'zh_Hant' and 'any', defaults to 'any'."
		)
		(
			"accoutname,n",
			value<std::string>(&auth_.name)->required(),
			BX_ACCOUNT_NAME
		)
		(
			"password,p",
			value<std::string>(&option_.passwd)->required(),
			BX_ACCOUNT_AUTH
		)
		(
			"hd_index,i",
			value<std::uint32_t>(&option_.hd_index),
			"Teh HD index for the account."
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
        argument()
          : words()
        {
        }
		std::vector<std::string> words;
    } argument_;

    struct option
    {
        option()
          : language(), passwd(""), hd_index(1)
        {
        }

        explorer::config::language language;
		std::string passwd;
		uint32_t hd_index;
    } option_;

};

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

