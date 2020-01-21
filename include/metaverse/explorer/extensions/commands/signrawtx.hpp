/**
 * Copyright (c) 2019-2020 mvs developers
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

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ signrawtx *************************/

class signrawtx: public command_extension
{
public:
    static const char* symbol() { return "signrawtx";}
    const char* name() override { return symbol();}
    bool category(int bs) override { return (ctgy_extension & bs ) == bs; }
    const char* description() override { return "signrawtx "; }

    arguments_metadata& load_arguments() override
    {
        return get_argument_metadata()
               .add("ACCOUNTNAME", 1)
               .add("ACCOUNTAUTH", 1)
               .add("TRANSACTION", 1);
    }

    void load_fallbacks (std::istream& input,
                         po::variables_map& variables) override
    {
        const auto raw = requires_raw_input();
        load_input(auth_.name, "ACCOUNTNAME", variables, input, raw);
        load_input(auth_.auth, "ACCOUNTAUTH", variables, input, raw);
        load_input(argument_.transaction, "TRANSACTION", variables, input, raw);
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
            "TRANSACTION",
            value<explorer::config::transaction>(&argument_.transaction)->required(),
            "The input Base16 transaction to sign."
        )
        (
            "wif,w",
            value<std::vector<std::string>>(&option_.private_keys),
            "The wif(s) or private key(s) to sign."
        )
        (
            "offline,o",
            value<bool>(&option_.offline)->default_value(false)->zero_tokens(),
            "If specified, then sign offline. Default is not specified."
        );

        return options;
    }

    void set_defaults_from_config (po::variables_map& variables) override
    {
    }

    console_result invoke (Json::Value& jv_output,
                           bc::server::server_node& node) override;

    struct argument
    {
        explorer::config::transaction transaction;
    } argument_;

    struct option
    {
        std::vector<std::string> private_keys;
        bool offline;
    } option_;

private:
    std::string get_private_key(blockchain::block_chain_impl& blockchain, const std::string& address);
    std::string get_private_key(const std::vector<std::string>& keys, const std::string& address);
    chain::script get_prev_output_script(blockchain::block_chain_impl& blockchain, const chain::input& input) const;

    bc::endorsement sign(
        const std::string& private_key,
        tx_type tx_,
        const uint32_t& index,
        const bc::explorer::config::script& config_contract,
        data_chunk& public_key_data);

};

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

