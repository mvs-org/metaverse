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
#pragma once

#include <metaverse/bitcoin.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/command.hpp>
#include <metaverse/explorer/extensions/command_extension.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ importaccount *************************/

class importaccount: public command_extension
{
public:
    static const char* symbol(){ return "importaccount";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
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
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
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
			"Account name."
		)
		(
			"password,p",
			value<std::string>(&option_.passwd)->required(),
			"Account password."
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

    console_result invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node) override;

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
          : language(), passwd(""), hd_index(0)
        {
        }

        explorer::config::language language;
		std::string passwd;
		uint32_t hd_index;
    } option_;

};

/************************ changepasswd *************************/

class changepasswd: public command_extension
{
public:
    static const char* symbol(){ return "changepasswd";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "changepasswd "; }

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
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        )
	    (
            "ACCOUNTNAME",
            value<std::string>(&auth_.name)->required(),
            "Account name."
	    )
        (
            "ACCOUNTAUTH",
            value<std::string>(&auth_.auth)->required(),
            "Account password/authorization."
	    )
		(
			"password,p",
			value<std::string>(&option_.passwd)->required(),
			"The new password."
		);

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }
		
    console_result invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node) override;

    struct argument
    {
    } argument_;

    struct option
    {
        option()
          : passwd()
        {
        }

        std::string passwd;
    } option_;

};

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
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        )
		(
			"ACCOUNTNAME",
			value<std::string>(&auth_.name)->required(),
			"Account name."
		)
		(
			"ACCOUNTAUTH",
			value<std::string>(&auth_.auth)->required(),
			"Account password/authorization."
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


/************************ listmultisig *************************/

class listmultisig: public command_extension
{
public:
    static const char* symbol(){ return "listmultisig";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "listmultisig "; }

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
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        )
		(
			"ACCOUNTNAME",
			value<std::string>(&auth_.name)->required(),
			"Account name."
		)
		(
			"ACCOUNTAUTH",
			value<std::string>(&auth_.auth)->required(),
			"Account password/authorization."
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
          : index(0)
        {
        }

		uint16_t index;
    } option_;

};


/************************ deletemultisig *************************/

class deletemultisig: public command_extension
{
public:
    static const char* symbol(){ return "deletemultisig";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "deletemultisig "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ACCOUNTNAME", 1)
            .add("ACCOUNTAUTH", 1)
            .add("ADDRESS", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "ACCOUNTNAME", variables, input, raw);
        load_input(auth_.auth, "ACCOUNTAUTH", variables, input, raw);
        load_input(option_.address, "ADDRESS", variables, input, raw);
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
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        )
		(
			"ACCOUNTNAME",
			value<std::string>(&auth_.name)->required(),
			"Account name."
		)
		(
			"ACCOUNTAUTH",
			value<std::string>(&auth_.auth)->required(),
			"Account password/authorization."
		)
		(
			"ADDRESS",
			value<std::string>(&option_.address)->required(),
			"The multisig script corresponding address."
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
          : address(""), m(0), n(0), index(0)
        {
        }
		uint16_t index;
		uint16_t m;
		uint16_t n;
		std::vector<std::string> public_keys;
		std::string self_publickey;
        std::string address;
		
    } option_;

};


} // commands
} // explorer
} // libbitcoin
