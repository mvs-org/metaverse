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


/************************ getnewmultisig *************************/

class getnewmultisig: public command_extension
{
public:
    static const char* symbol(){ return "getnewmultisig";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getnewmultisig "; }

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
			"signaturenum,m",
			value<uint16_t>(&option_.m)->required(),
			"Account multisig signature number."
		)
		(
			"publickeynum,n",
			value<uint16_t>(&option_.n)->required(),
			"Account multisig public key number."
		)
		(
			"selfpublickey,s",
			value<std::string>(&option_.self_publickey)->required(),
			"the public key belongs to this account."
		)
		(
			"publickey,k",
			value<std::vector<std::string>>(&option_.public_keys),
			"cosigner public key used for multisig"
		)
		(
			"description,d",
			value<std::string>(&option_.description),
			"multisig record description."
		)
		;

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node) override;

    struct argument
    {
        argument()
        {
        }
    } argument_;

    struct option
    {
        option()
          : self_publickey(""), description(""), m(0), n(0)
        {
        }

		uint16_t m;
		uint16_t n;
		std::vector<std::string> public_keys;
		std::string self_publickey;
		std::string description;
		
    } option_;

};

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

