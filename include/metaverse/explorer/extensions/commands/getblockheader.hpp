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


#pragma once
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/extensions/command_extension.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/config/hashtype.hpp>
#include <metaverse/explorer/config/header.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ getblockheader *************************/

class getblockheader: public command_extension
{
public:
    getblockheader() noexcept {};
    getblockheader(const std::string& other){ if (other == "getbestblockhash") option_.is_getbestblockhash = true; }
    static const char* symbol(){ return "getblockheader";}
    const char* name() override { return symbol();}
    bool category(int bs) override { return (ctgy_extension & bs ) == bs; }
    const char* description() override { return "getblockheader, alias as fetch-header/getbestblockhash/getbestblockheader."; }

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
            "hash,s",
            value<bc::config::hash256>(&option_.hash),
            "The Base16 block hash."
        )
        (
            "height,t",
            value<uint32_t>(&option_.height),
            "The block height."
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
        option():
            hash(),
            height(std::numeric_limits<uint32_t>::max())
        {}

        bool is_getbestblockhash{false};
	    bc::config::hash256 hash;
        uint32_t height;
    } option_;

};




} // namespace commands
} // namespace explorer
} // namespace libbitcoin

