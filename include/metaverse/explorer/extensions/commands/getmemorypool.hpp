/*
 * getmemorypool.hpp
 *
 *  Created on: Jul 3, 2017
 *      Author: jiang
 */

#ifndef INCLUDE_METAVERSE_EXPLORER_EXTENSIONS_WALLET_GETMEMORYPOOL_HPP_
#define INCLUDE_METAVERSE_EXPLORER_EXTENSIONS_WALLET_GETMEMORYPOOL_HPP_



#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/extensions/command_extension.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ getbalance *************************/

class getmemorypool: public command_extension
{
public:
    static const char* symbol(){ return "getmemorypool";}
    const char* name() override { return symbol();}
    const char* category() override { return "EXTENSION"; }
    const char* description() override { return "Returns all transactions in memory pool."; }

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
            "json,j",
            value<bool>(&option_.json)->default_value(true),
            "Json format or Raw format, default is Json(true)."
        )
	    (
            "ACCOUNTNAME",
            value<std::string>(&auth_.name),
            BX_ACCOUNT_NAME
	    )
        (
            "ACCOUNTAUTH",
            value<std::string>(&auth_.auth),
            BX_ACCOUNT_AUTH
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
        bool json;
    } option_;

};


} // namespace commands
} // namespace explorer
} // namespace libbitcoin




#endif /* INCLUDE_METAVERSE_EXPLORER_EXTENSIONS_WALLET_GETMEMORYPOOL_HPP_ */
