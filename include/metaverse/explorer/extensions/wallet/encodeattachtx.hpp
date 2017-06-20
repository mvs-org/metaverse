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
        std::ostream& cerr, libbitcoin::server::server_node& node) override;

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

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

