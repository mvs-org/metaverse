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

#include <boost/property_tree/ptree.hpp>      
#include <boost/property_tree/json_parser.hpp>

#include <metaverse/bitcoin.hpp>
#include <metaverse/client.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/callback_state.hpp>
#include <metaverse/explorer/display.hpp>
#include <metaverse/explorer/prop_tree.hpp>
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/command_extension.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."
/************************ submitwork *************************/

class submitwork: public command_extension
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
        std::ostream& cerr, libbitcoin::server::server_node& node) override;

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


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

