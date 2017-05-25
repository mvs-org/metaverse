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

#include <functional>
#include <memory>
#include <string>
#include <metaverse/bitcoin.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/command.hpp>
#include <metaverse/bitcoin/chain/attachment/asset/asset_detail.hpp>  // used for createasset
#include <metaverse/explorer/config/metaverse_output.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

struct prikey_amount{
    std::string first;
    uint64_t    second;
};

struct prikey_etp_amount{
    std::string key;
    uint64_t    value;
    uint64_t    asset_amount;
    output_point output;
};

struct utxo_attach_info {
	//target:1:asset-transfer:symbol:amount
	std::string target;
    uint32_t version;
    std::string type;
	std::string symbol;
    uint64_t amount;
	uint64_t value; // etp
	std::string output_option; // used by get_tx_encode
};

template<class T1, class T2>
class BCX_API colon_delimited2_item
{
public:

    /**
     * Default constructor.
     */
	colon_delimited2_item()
	{
	};
	
    colon_delimited2_item(T1 first, T2 second)
	: first_(first), second_(second)
	{
	};
    
    /**
     * Initialization constructor.
     * @param[in]  tuple  The value to initialize with.
     */
    colon_delimited2_item(const std::string& tuple)
    {
	    std::stringstream(tuple) >> *this;
	};

	
	static bool decode_colon_delimited(colon_delimited2_item<T1, T2>& height, const std::string& tuple)
	{
		const auto tokens = split(tuple, BX_TX_POINT_DELIMITER);
		if (tokens.size() != 2)
			return false;
	
		deserialize(height.first_, tokens[0], true);
		deserialize(height.second_, tokens[1], true);
	
		return true;
	};
	
	// colon_delimited2_item is currently a private encoding in bx.
	static std::string encode_colon_delimited(const colon_delimited2_item<T1, T2>& height)
	{
		std::stringstream result;
		result << height.first_ << BX_TX_POINT_DELIMITER <<
			height.second_;
		return result.str();
	};

    /**
     * Overload stream in. Throws if colon_delimited2_item is invalid.
     * @param[in]   colon_delimited2_item     The colon_delimited2_item stream to read the value from.
     * @param[out]  argument  The object to receive the read value.
     * @return                The colon_delimited2_item stream reference.
     */
    friend std::istream& operator>>(std::istream& stream,
        colon_delimited2_item& argument)
    {
        
	    std::string tuple;
	    stream >> tuple;

	    if (!decode_colon_delimited(argument, tuple))
	    {
	        throw std::logic_error{"invalid option " + tuple};
	    }

	    return stream;
	};

    /**
     * Overload stream out.
     * @param[in]   output    The output stream to write the value to.
     * @param[out]  argument  The object from which to obtain the value.
     * @return                The output stream reference.
     */
    friend std::ostream& operator<<(std::ostream& output,
        const colon_delimited2_item& argument)
    {
	    output << encode_colon_delimited(argument);
	    return output;
	};
	// get method
	T1 first(){
		return first_;
	};
	
	void set_first(const T1& first){
		first_ = first;
	};
	
	T2 second(){
		return second_;
	};

	void set_second(const T2& second){
		second_ = second;
	};
	
private:
	/**
     * The state of this object. only for uint64_t
     */
    T1 first_;
    T2 second_;
};

class command_extension:public command{
protected:
    struct argument_base
    {
        std::string name;
        std::string auth;
    } auth_;
};

/************************ setminingaccount *************************/

class setminingaccount: public command_extension
{
public:
    static const char* symbol(){ return "setminingaccount";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "setminingaccount when pool mining."; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ACCOUNTNAME", 1)
            .add("ACCOUNTAUTH", 1)
            .add("PAYMENT_ADDRESS", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "ACCOUNTNAME", variables, input, raw);
        load_input(auth_.auth, "ACCOUNTAUTH", variables, input, raw);
        load_input(auth_.auth, "PAYMENT_ADDRESS", variables, input, raw);
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
            "PAYMENT_ADDRESS",
            value<bc::wallet::payment_address>(&argument_.payment_address)->required(),
            "the payment address of this account."
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain,
        bc::consensus::miner& miner
        ) override;

    struct argument
    {
        bc::wallet::payment_address payment_address;
    } argument_;

    struct option
    {
    } option_;

};

/************************ stopall *************************/

class stopall: public command_extension
{
public:
    static const char* symbol(){ return "stopall";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "stop mvsd."; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ADMINNAME", 1)
            .add("ADMINAUTH", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "ADMINNAME", variables, input, raw);
        load_input(auth_.auth, "ADMINAUTH", variables, input, raw);
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
            "ADMINNAME",
            value<std::string>(&auth_.name)->required(),
            "admin name."
	    )
        (
            "ADMINAUTH",
            value<std::string>(&auth_.auth)->required(),
            "admin password/authorization."
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain
        ) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ stop *************************/

class stop: public command_extension
{
public:
    static const char* symbol(){ return "stop";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "stop solo mining."; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain,
        bc::consensus::miner& miner
        ) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};

/************************ getwork *************************/

class getwork: public command
{
public:
    static const char* symbol(){ return "getwork";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getwork to get mining info"; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata();
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
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
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain,
        bc::consensus::miner& miner
        ) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};

/************************ submitwork *************************/

class submitwork: public command
{
public:
    static const char* symbol(){ return "submitwork";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "submitwork to submit mining result."; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("NOUNCE", 1)
            .add("HEADERHASH", 1)
            .add("MIXHASH", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(argument_.nounce, "NOUNCE", variables, input, raw);
        load_input(argument_.header_hash, "HEADERHASH", variables, input, raw);
        load_input(argument_.mix_hash, "MIXHASH", variables, input, raw);
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
            "NOUNCE",
            value<std::string>(&argument_.nounce)->required(),
            "nounce."
        )
        (
            "HEADERHASH",
            value<std::string>(&argument_.header_hash)->required(),
            "header hash."
        )
        (
            "MIXHASH",
            value<std::string>(&argument_.mix_hash)->required(),
            "mix hash."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain,
        bc::consensus::miner& miner
        ) override;

    struct argument
    {
        std::string nounce;     
        std::string mix_hash;   
        std::string header_hash;
    } argument_;

    struct option
    {
    } option_;

};

/************************ submithashrate *************************/

class submithashrate: public command
{
public:
    static const char* symbol(){ return "submithashrate";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "submithashrate not used."; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("RATE", 1)
            .add("RANDOM", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(argument_.rate, "RATE", variables, input, raw);
        load_input(argument_.random, "RANDOM", variables, input, raw);
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
            "RATE",
            value<std::string>(&argument_.rate)->required(),
            "rate."
	    )
        (
            "RANDOM",
            value<std::string>(&argument_.random)->required(),
            "random."
	    );


        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain,
        bc::consensus::miner& miner
        ) override;

    struct argument
    {
        std::string rate;
        std::string random;
    } argument_;

    struct option
    {
    } option_;

};


/************************ start *************************/

class start: public command_extension
{
public:
    static const char* symbol(){ return "start";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "start solo mining."; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain,
        bc::consensus::miner& miner
        ) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};

/************************ getinfo *************************/

class getinfo: public command_extension
{
public:
    static const char* symbol(){ return "getinfo";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getinfo "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getpeerinfo *************************/

class getpeerinfo: public command_extension
{
public:
    static const char* symbol(){ return "getpeerinfo";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getpeerinfo "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ ping *************************/

class ping: public command_extension
{
public:
    static const char* symbol(){ return "ping";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "ping "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ addnode *************************/

class addnode: public command_extension
{
public:
    static const char* symbol(){ return "addnode";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "addnode "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getmininginfo *************************/

class getmininginfo: public command_extension
{
public:
    static const char* symbol(){ return "getmininginfo";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getmininginfo "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

	console_result invoke (std::ostream& output,
			std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain,
			bc::consensus::miner& miner) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};

/************************ fetchheaderext *************************/

class fetchheaderext: public command_extension
{
public:
    static const char* symbol(){ return "fetchheaderext";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "fetchheaderext "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ACCOUNTNAME", 1)
            .add("ACCOUNTAUTH", 1)
            .add("NUMBER", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "ACCOUNTNAME", variables, input, raw);
        load_input(auth_.auth, "ACCOUNTAUTH", variables, input, raw);
        load_input(argument_.number, "NUMBER", variables, input, raw);
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
            "NUMBER",
            value<std::string>(&argument_.number)->required(),
            "Block number, or earliest, latest or pending"
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

	console_result invoke (std::ostream& output,
			std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain,
			bc::consensus::miner& miner) override;

    struct argument
    {
    	std::string number;
    } argument_;

    struct option
    {
    } option_;

};


/************************ gettransaction *************************/

class gettransaction: public command_extension
{
public:
    static const char* symbol(){ return "gettransaction";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "gettransaction "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("HASH", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(argument_.hash, "HASH", variables, input, raw);
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
            "HASH",
            value<bc::config::hash256>(&argument_.hash)->required(),
            "The Base16 transaction hash of the transaction to get. If not specified the transaction hash is read from STDIN."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

	console_result invoke (std::ostream& output,
			std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
        bc::config::hash256 hash;
    } argument_;

    struct option
    {
    } option_;

};

/************************ backupwallet *************************/

class backupwallet: public command_extension
{
public:
    static const char* symbol(){ return "backupwallet";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "backupwallet "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ importwallet *************************/

class importwallet: public command_extension
{
public:
    static const char* symbol(){ return "importwallet";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "importwallet "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ lockwallet *************************/

class lockwallet: public command_extension
{
public:
    static const char* symbol(){ return "lockwallet";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "lockwallet "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ backupaccount *************************/

class backupaccount: public command_extension
{
public:
    static const char* symbol(){ return "backupaccount";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "backupaccount "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



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
		);

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

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

/************************ listaccounts *************************/

class listaccounts: public command_extension
{
public:
    static const char* symbol(){ return "listaccounts";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "listaccounts "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getnewaccount *************************/

class getnewaccount: public command_extension
{
public:
    static const char* symbol(){ return "getnewaccount";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "Generate a new account from this wallet."; }

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
            "language,l",
            value<std::string>(&option_.language)->default_value("en"),
            "Options are 'en', 'es', 'ja', 'zh_Hans', 'zh_Hant' and 'any', defaults to 'en'."
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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
        std::string language;
    } option_;

};



/************************ getaccount *************************/

class getaccount: public command_extension
{
public:
    static const char* symbol(){ return "getaccount";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "Show account details"; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ACCOUNTNAME", 1)
            .add("ACCOUNTAUTH", 1)
            .add("LASTWORD", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "ACCOUNTNAME", variables, input, raw);
        load_input(auth_.auth, "ACCOUNTAUTH", variables, input, raw);
        load_input(auth_.auth, "LASTWORD", variables, input, raw);
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
            "LASTWORD",
            value<std::string>(&argument_.last_word)->required(),
            "The last word of your master private-key phrase."
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
        std::string last_word;
    } argument_;

    struct option
    {
    } option_;

};

/************************ deleteaccount *************************/

class deleteaccount: public command_extension
{
public:
    static const char* symbol(){ return "deleteaccount";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "deleteaccount "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ACCOUNTNAME", 1)
            .add("ACCOUNTAUTH", 1)
            .add("LASTWORD", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "ACCOUNTNAME", variables, input, raw);
        load_input(auth_.auth, "ACCOUNTAUTH", variables, input, raw);
        load_input(auth_.auth, "LASTWORD", variables, input, raw);
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
            "LASTWORD",
            value<std::string>(&argument_.last_word)->required(),
            "The last word of your private-key phrase."
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
        std::string last_word;
    } argument_;

    struct option
    {
    } option_;

};


/************************ lockaccount *************************/

class lockaccount: public command_extension
{
public:
    static const char* symbol(){ return "lockaccount";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "lockaccount "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ setaccountinfo *************************/

class setaccountinfo: public command_extension
{
public:
    static const char* symbol(){ return "setaccountinfo";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "setaccountinfo "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ listaddresses *************************/

class listaddresses: public command_extension
{
public:
    static const char* symbol(){ return "listaddresses";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "List available addresses of this account."; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getnewaddress *************************/

class getnewaddress: public command_extension
{
public:
    static const char* symbol(){ return "getnewaddress";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "Generate new address for this account."; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getaddress *************************/

class getaddress: public command_extension
{
public:
    static const char* symbol(){ return "getaddress";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getaddress "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getblock *************************/

class getblock: public command_extension
{
public:
    static const char* symbol(){ return "getblock";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "Get sepcified block header from wallet."; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
        		.add("hash", 1)
        		.add("json", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.auth, "hash", variables, input, raw);
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
				"hash",
				value<bc::config::hash256>(&argument_.hash)->required(),
				"block hash."
		)
		(
				"json",
				value<bool>(&argument_.json),
				"use json format or not, default is false"
		)
		;

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    	bc::config::hash256 hash;
    	bool json;
    } argument_;

    struct option
    {
    } option_;

};



/************************ signmessage *************************/

class signmessage: public command_extension
{
public:
    static const char* symbol(){ return "signmessage";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "signmessage "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ verifymessage *************************/

class verifymessage: public command_extension
{
public:
    static const char* symbol(){ return "verifymessage";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "verifymessage "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ createmultisig *************************/

class createmultisig: public command_extension
{
public:
    static const char* symbol(){ return "createmultisig";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "createmultisig "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ addmultisigaddress *************************/

class addmultisigaddress: public command_extension
{
public:
    static const char* symbol(){ return "addmultisigaddress";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "addmultisigaddress "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ validateaddress *************************/

class validateaddress: public command_extension
{
public:
    static const char* symbol(){ return "validateaddress";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "validateaddress "; }

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
			value<std::string>(&argument_.address)->required(),
			"address"
		);

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    	std::string address;
    } argument_;

    struct option
    {
    } option_;

};



/************************ listbalances *************************/

class listbalances: public command_extension
{
public:
    static const char* symbol(){ return "listbalances";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "List all balance details of each address of this account."; }

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
            "nozero,n",
            value<bool>(&option_.non_zero)->zero_tokens()->default_value(false),
            "List non-zero upsent records."
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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
        bool non_zero;
    } option_;

};



/************************ getbalance *************************/

class getbalance: public command_extension
{
public:
    static const char* symbol(){ return "getbalance";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "Show total balance details of this account."; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getbestblockhash *************************/

class getbestblockhash: public command
{
public:
    static const char* symbol(){ return "getbestblockhash";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getbestblockhash "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata();
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
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
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getbestblockheader *************************/

class getbestblockheader: public command
{
public:
    static const char* symbol(){ return "getbestblockheader";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getbestblockheader "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata();
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
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
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ listtxs *************************/

class listtxs: public command_extension
{
public:
    static const char* symbol(){ return "listtxs";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
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
            "address,a",
            value<std::string>(&argument_.address),
            "Address."
	    )
	    (
            "height,e",
            value<libbitcoin::explorer::commands::colon_delimited2_item<uint64_t, uint64_t>>(&option_.height),
            "Get tx according height eg: -e start-height:end-height."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    	std::string address;
    } argument_;

    struct option
    {
    	option():height(0, 0)
		{};
    	libbitcoin::explorer::commands::colon_delimited2_item<uint64_t, uint64_t> height;
    } option_;

};



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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getaddresstx *************************/

class getaddresstx: public command_extension
{
public:
    static const char* symbol(){ return "getaddresstx";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getaddresstx "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getaccounttx *************************/

class getaccounttx: public command_extension
{
public:
    static const char* symbol(){ return "getaccounttx";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getaccounttx "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};
#if 0
/************************ deposit *************************/

class deposit : public command_extension
{
public:
    static const char* symbol(){ return "deposit";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "Deposit some etp, then get reward for frozen some etp."; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ACCOUNTNAME", 1)
            .add("ACCOUNTAUTH", 1)
            .add("AMOUNT", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "ACCOUNTNAME", variables, input, raw);
        load_input(auth_.auth, "ACCOUNTAUTH", variables, input, raw);
        load_input(argument_.amount, "AMOUNT", variables, input, raw);
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
            "AMOUNT",
            value<uint64_t>(&argument_.amount)->required(),
            "How many you will deposit."
        )
		(
			"address,a",
			value<std::string>(&argument_.address),
			"The deposit target address."
		)
	    (
            "deposit,d",
            value<uint16_t>(&argument_.deposit)->default_value(7),
            "Deposits support [7, 30, 90, 182, 365] days. defaluts to 7 days"
	    )
	    (
            "fee,f",
            value<uint64_t>(&argument_.fee)->default_value(10000),
            "The fee of tx. default_value 0.0001 etp"
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
        uint64_t amount;
        uint64_t fee;
        uint16_t deposit;
		std::string address;
    } argument_;

    struct option
    {
    } option_;

};
#endif

class send_command: public command_extension{
public:
	virtual bool is_block_height_fullfilled(uint64_t height) override{
		if (height >= 200000) {
			return true;
		}
		return false;
	}

	virtual uint64_t minimum_block_height() override{
		return 200000;
	}
};

#if 0
/************************ send *************************/

class send: public send_command
{
public:
    static const char* symbol(){ return "send";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "send etp to a targert address, mychange goes to another existed address of this account."; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ACCOUNTNAME", 1)
            .add("ACCOUNTAUTH", 1)
            .add("TOADDRESS", 1)
            .add("AMOUNT", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "ACCOUNTNAME", variables, input, raw);
        load_input(auth_.auth, "ACCOUNTAUTH", variables, input, raw);
        load_input(argument_.address, "TOADDRESS", variables, input, raw);
        load_input(argument_.amount, "AMOUNT", variables, input, raw);
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
            "TOADDRESS",
            value<std::string>(&argument_.address)->required(),
            "Send to this address"
	    )
        (
            "AMOUNT",
            value<uint64_t>(&argument_.amount)->required(),
            "How many you will spend"
        )
	    (
            "fee,f",
            value<uint64_t>(&argument_.fee)->default_value(10000),
            "The fee of tx. default_value 0.0001 etp"
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
        std::string address;
        uint64_t amount;
        uint64_t fee;
    } argument_;

    struct option
    {
    } option_;

};



/************************ sendmore *************************/

class sendmore: public send_command
{
public:
    static const char* symbol(){ return "sendmore";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "send etp to multi target addresses, can specify mychange address. Eg: [sendmore $name $password -r $address1:$amount1 -r $address2:$amount2 -m $mychange_address]"; }

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
            "Send to more target. "
        )
        (
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "Get a description and instructions for this command."
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
            "receivers,r",
            value<std::vector<std::string>>(&argument_.receivers)->required(),
            "Send to [address:amount]."
	    )
        (
            "mychange,m",
            value<std::string>(&argument_.mychange_address),
            "Mychange to this address"
	    )
	    (
            "fee,f",
            value<uint64_t>(&argument_.fee)->default_value(10000),
            "The fee of tx. default_value 0.0001 etp"
	    );


        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
        std::vector<std::string> receivers;
        std::string mychange_address;
        uint64_t fee;
    } argument_;

    struct option
    {
    } option_;

};



/************************ sendfrom *************************/

class sendfrom: public send_command
{
public:
    static const char* symbol(){ return "sendfrom";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "send etp from a specified address of this account to target address, mychange goes to from_address."; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ACCOUNTNAME", 1)
            .add("ACCOUNTAUTH", 1)
            .add("FROMADDRESS", 1)
            .add("TOADDRESS", 1)
            .add("AMOUNT", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "ACCOUNTNAME", variables, input, raw);
        load_input(auth_.auth, "ACCOUNTAUTH", variables, input, raw);
        load_input(auth_.auth, "FROMADDRESS", variables, input, raw);
        load_input(auth_.auth, "TOADDRESS", variables, input, raw);
        load_input(auth_.auth, "AMOUNT", variables, input, raw);
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
			"FROMADDRESS",
			value<std::string>(&argument_.from)->required(),
			"Send from this address"
		)
		(
			"TOADDRESS",
			value<std::string>(&argument_.to)->required(),
			"Send to this address"
		)
		(
			"AMOUNT",
			value<uint64_t>(&argument_.amount)->required(),
			"How many you will spend"
		)
		(
			"fee,f",
			value<uint64_t>(&argument_.fee)->default_value(10000),
			"The fee of tx. default_value 0.0001 etp"
		);

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    	std::string from;
		std::string to;
		uint64_t amount;
		uint64_t fee;
    } argument_;

    struct option
    {
    } option_;

};
#endif
#if 0

/************************ sendwithmsg *************************/

class sendwithmsg: public send_command
{
public:
    static const char* symbol(){ return "sendwithmsg";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "sendwithmsg "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ sendwithmsgfrom *************************/

class sendwithmsgfrom: public send_command
{
public:
    static const char* symbol(){ return "sendwithmsgfrom";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "sendwithmsgfrom "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};

#endif

/************************ listassets *************************/

class listassets: public command_extension
{
public:
    static const char* symbol(){ return "listassets";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "list assets details."; }

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
            value<std::string>(&auth_.name),
            "Account name."
	    )
        (
            "ACCOUNTAUTH",
            value<std::string>(&auth_.auth),
            "Account password/authorization."
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ getasset *************************/

class getasset: public command_extension
{
public:
    static const char* symbol(){ return "getasset";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getasset "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ACCOUNTNAME", 1)
            .add("ACCOUNTAUTH", 1)
            .add("SYMBOL", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "ACCOUNTNAME", variables, input, raw);
        load_input(auth_.auth, "ACCOUNTAUTH", variables, input, raw);
        load_input(argument_.symbol, "SYMBOL", variables, input, raw);
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
			"SYMBOL",
			value<std::string>(&argument_.symbol)->required(),
			"Asset symbol."
		);

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    	argument():
			symbol()
		{
		}

		std::string symbol;
    } argument_;

    struct option
    {
    } option_;

};



/************************ getaddressasset *************************/

class getaddressasset: public command_extension
{
public:
    static const char* symbol(){ return "getaddressasset";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getaddressasset "; }

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
			value<std::string>(&argument_.address)->required(),
			"address"
		);

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    	std::string address;
    } argument_;

    struct option
    {
    } option_;

};



/************************ getaccountasset *************************/

class getaccountasset: public command_extension
{
public:
    static const char* symbol(){ return "getaccountasset";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getaccountasset "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ createasset *************************/

class createasset: public command_extension
{
public:
    static const char* symbol(){ return "createasset";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
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
            "symbol,s",
            value<std::string>(&option_.symbol)->required(),
            "The asset symbol/name. Global unique."
        )
        (
            "volume,v",
            value<uint64_t>(&option_.maximum_supply)->required(),
            "The asset maximum supply volume."
        )
        (
            "type,t",
            value<uint32_t>(&option_.asset_type),
            "The asset type."
        )
        (
            "issuer,i",
            value<std::string>(&option_.issuer),
            "The asset issuer.defaults to account name."
        )
        (
            "address,a",
            value<std::string>(&option_.address),
            "The asset will be issued from which address."
        )
        (
            "description,d",
            value<std::string>(&option_.description),
            "The asset description."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    	option()
		  : symbol(),
			maximum_supply(),
			asset_type(),
			issuer(),
			address(),
			description()
    	{
    	}
		
		std::string symbol;
		uint64_t maximum_supply;
		uint32_t asset_type;
		std::string issuer; 
		std::string address;
		std::string description;
    } option_;

};



/************************ issue *************************/

class issue: public command_extension
{
public:
    static const char* symbol(){ return "issue";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "issue "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ACCOUNTNAME", 1)
            .add("ACCOUNTAUTH", 1)
            .add("SYMBOL", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "ACCOUNTNAME", variables, input, raw);
        load_input(auth_.auth, "ACCOUNTAUTH", variables, input, raw);
        load_input(argument_.symbol, "SYMBOL", variables, input, raw);
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
			"SYMBOL",
			value<std::string>(&argument_.symbol)->required(),
			"issued asset symbol"
		)
		(
			"fee,f",
			value<uint64_t>(&argument_.fee)->default_value(1000000000),
			"The fee of tx. default_value 10 etp"
		);

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    	std::string symbol;
    	uint64_t fee;
    } argument_;

    struct option
    {
    } option_;

};



/************************ issuefrom *************************/

class issuefrom: public command_extension
{
public:
    static const char* symbol(){ return "issuefrom";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "issuefrom "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ACCOUNTNAME", 1)
            .add("ACCOUNTAUTH", 1)
            .add("ADDRESS", 1)
            .add("SYMBOL", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "ACCOUNTNAME", variables, input, raw);
        load_input(auth_.auth, "ACCOUNTAUTH", variables, input, raw);
        load_input(argument_.address, "ADDRESS", variables, input, raw);
        load_input(argument_.symbol, "SYMBOL", variables, input, raw);
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
			value<std::string>(&argument_.address)->required(),
			"target address"
		)
		(
			"SYMBOL",
			value<std::string>(&argument_.symbol)->required(),
			"issued asset symbol"
		)
		(
			"fee,f",
			value<uint64_t>(&argument_.fee)->default_value(1000000000),
			"The fee of tx. default_value 10 etp"
		);

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    	std::string address;
    	std::string symbol;
    	uint64_t fee;
    } argument_;

    struct option
    {
    } option_;

};



/************************ issuemore *************************/

class issuemore: public command_extension
{
public:
    static const char* symbol(){ return "issuemore";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "issuemore "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ issuemorefrom *************************/

class issuemorefrom: public command_extension
{
public:
    static const char* symbol(){ return "issuemorefrom";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "issuemorefrom "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ sendasset *************************/

class sendasset: public command_extension
{
public:
    static const char* symbol(){ return "sendasset";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "sendasset "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ACCOUNTNAME", 1)
            .add("ACCOUNTAUTH", 1)
			.add("ADDRESS", 1)
			.add("SYMBOL", 1)
			.add("AMOUNT", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "ACCOUNTNAME", variables, input, raw);
        load_input(auth_.auth, "ACCOUNTAUTH", variables, input, raw);
        load_input(argument_.address, "ADDRESS", variables, input, raw);
        load_input(argument_.symbol, "SYMBOL", variables, input, raw);
        load_input(argument_.amount, "AMOUNT", variables, input, raw);
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
			value<std::string>(&argument_.address)->required(),
			"Asset receiver."
		)
		(
			"SYMBOL",
			value<std::string>(&argument_.symbol)->required(),
			"Asset symbol/name."
		)
		(
			"AMOUNT",
			value<uint64_t>(&argument_.amount)->required(),
			"Asset count."
		)
	    (
            "fee,f",
            value<uint64_t>(&argument_.fee)->default_value(10000),
            "The fee of tx. default_value 0.0001 etp"
	    );
        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    	std::string address;
		std::string symbol;
    	uint64_t amount;
    	uint64_t fee;
    } argument_;

    struct option
    {
    } option_;

};



/************************ sendassetfrom *************************/

class sendassetfrom: public command_extension
{
public:
    static const char* symbol(){ return "sendassetfrom";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "sendassetfrom "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
            .add("ACCOUNTNAME", 1)
            .add("ACCOUNTAUTH", 1)
            .add("FROMADDRESS", 1)
            .add("TOADDRESS", 1)
            .add("SYMBOL", 1)
            .add("AMOUNT", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "ACCOUNTNAME", variables, input, raw);
        load_input(auth_.auth, "ACCOUNTAUTH", variables, input, raw);
        load_input(argument_.from, "FROMADDRESS", variables, input, raw);
        load_input(argument_.to, "TOADDRESS", variables, input, raw);
        load_input(argument_.symbol, "SYMBOL", variables, input, raw);
        load_input(argument_.amount, "AMOUNT", variables, input, raw);
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
			"FROMADDRESS",
			value<std::string>(&argument_.from)->required(),
			"from address"
		)
		(
			"TOADDRESS",
			value<std::string>(&argument_.to)->required(),
			"target address"
		)
		(
			"SYMBOL",
			value<std::string>(&argument_.symbol)->required(),
			"asset symbol"
		)
		(
			"AMOUNT",
			value<uint64_t>(&argument_.amount)->required(),
			"The asset amount shares"
		)
		(
			"fee,f",
			value<uint64_t>(&argument_.fee)->default_value(10000),
			"The fee of tx. default_value 0.0001 etp"
		);

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
		std::string from;
		std::string to;
		std::string symbol;
		uint64_t amount;
		uint64_t fee;
    } argument_;

    struct option
    {
    } option_;

};



/************************ getdid *************************/

class getdid: public command_extension
{
public:
    static const char* symbol(){ return "getdid";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getdid "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ setdid *************************/

class setdid: public command_extension
{
public:
    static const char* symbol(){ return "setdid";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "setdid "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ sendwithdid *************************/

class sendwithdid: public command_extension
{
public:
    static const char* symbol(){ return "sendwithdid";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "sendwithdid "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};



/************************ settxfee *************************/

class settxfee: public command_extension
{
public:
    static const char* symbol(){ return "settxfee";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "settxfee "; }

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
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
    } option_;

};

/************************ encodeattachtx *************************/
#define BX_ENCODEATTACHTX_INVALID_OUTPUT \
    "An output is not valid."
#define BX_ENCODEATTACHTX_LOCKTIME_CONFLICT \
    "The specified lock time is ineffective because all sequences are set to the maximum value."

using namespace  bc::explorer::config;

class encodeattachtx: public command_extension
{
public:
    static const char* symbol(){ return "encodeattachtx";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "encodeattachtx "; }

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
			"script_version,s",
			value<explorer::config::byte>(&option_.script_version)->default_value(5),
			"The pay-to-script-hash payment address version, defaults to 5. This is used to differentiate output addresses."
		)
		(
			"lock_time,l",
			value<uint32_t>(&option_.lock_time),
			"The transaction lock time."
		)
		(
			"version,v",
			value<uint32_t>(&option_.version)->default_value(1),
			"The transaction version, defaults to 1."
		)
		(
			"input,i",
			value<std::vector<explorer::config::input>>(&option_.inputs),
			"The set of transaction input points encoded as TXHASH:INDEX:SEQUENCE. TXHASH is a Base16 transaction hash. INDEX is the 32 bit input index in the context of the transaction. SEQUENCE is the optional 32 bit input sequence and defaults to the maximum value."
		)
		(
			"output,o",
			value<std::vector<explorer::config::metaverse_output>>(&option_.outputs),
			"The set of transaction output data encoded as TARGET:SATOSHI:SEED. TARGET is an address (including stealth or pay-to-script-hash) or a Base16 script. SATOSHI is the 32 bit spend amount in satoshi. SEED is required for stealth outputs and not used otherwise. The same seed should NOT be used for multiple outputs."
		);

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }
	
	bool push_scripts(std::vector<tx_output_type>& outputs,
		const explorer::config::metaverse_output& output, uint8_t script_version);
	
	void refill_output_attach(std::vector<explorer::config::metaverse_output>& vec_cfg_output,
			bc::blockchain::block_chain_impl& blockchain);
	
    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
        option()
          : script_version(),
            lock_time(),
            version(),
            inputs(),
            outputs()
        {
        }

        explorer::config::byte script_version;
        uint32_t lock_time;
        uint32_t version;
        std::vector<explorer::config::input> inputs;
        std::vector<explorer::config::metaverse_output> outputs;
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
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

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

/************************ changepasswdext *************************/

class changepasswdext: public command_extension
{
public:
    static const char* symbol(){ return "changepasswdext";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "changepasswdext "; }

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
            "accoutname,n",
            value<std::string>(&auth_.name),
            "Account name."
	    )
		(
			"language,l",
			value<explorer::config::language>(&option_.language),
			"The language identifier of the dictionary of the mnemonic. Options are 'en', 'es', 'ja', 'zh_Hans', 'zh_Hant' and 'any', defaults to 'any'."
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
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

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
          : language()
        {
        }

        explorer::config::language language;
		std::string passwd;
    } option_;

};

/************************ getbusinessbytime *************************/

class getbusinessbytime: public command_extension
{
public:
    static const char* symbol(){ return "getbusinessbytime";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "getbusinessbytime "; }

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
			"type,t",
			value<uint32_t>(&option_.type)->required(),
			"The business type."
		)
		(
			"start,s",
			value<uint32_t>(&option_.start)->required(),
			"The start timestamp."
		)
		(
			"end,e",
			value<uint32_t>(&option_.end)->required(),
			"The end timestamp."
		);

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }
		
    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    } argument_;

    struct option
    {
        option()
          : type()
        {
        }

        uint32_t type;
		uint32_t start;
		uint32_t end;
    } option_;

};

/************************ xfetchbalance *************************/

class xfetchbalance: public command_extension
{
public:
    static const char* symbol(){ return "xfetchbalance";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "xfetchbalance "; }

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
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        )
        (
            "ADDRESS",
            value<std::string>(&argument_.address)->required(),
            "Address."
	    )
	    (
            "type,t",
            value<std::string>(&option_.type)->default_value("all"),
            "UTXO type, support types : etp/all. default value is all"
	    );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    	std::string address;
    } argument_;

    struct option
    {
    	std::string type;
    } option_;

};

/************************ xfetchutxo *************************/

class xfetchutxo: public command_extension
{
public:
    static const char* symbol(){ return "xfetchutxo";}
    const char* name() override { return symbol();} 
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "xfetchutxo "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
			.add("AMOUNT", 1)
            .add("ADDRESS", 1);
    }

    void load_fallbacks (std::istream& input, 
        po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(argument_.address, "AMOUNT", variables, input, raw);
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
            BX_CONFIG_VARIABLE ",c",
            value<boost::filesystem::path>(),
            "The path to the configuration settings file."
        )
		(
			"AMOUNT",
			value<uint64_t>(&argument_.amount)->required(),
			"Amount."
		)
        (
            "ADDRESS",
            value<std::string>(&argument_.address)->required(),
            "Address."
	    )
		(
			"type,t",
			value<std::string>(&option_.type)->default_value("all"),
			"UTXO type, support types : etp/all. default value is all"
		);

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain) override;

    struct argument
    {
    	uint64_t amount;
    	std::string address;
    } argument_;

    struct option
    {
    	std::string type;
    } option_;

};


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

