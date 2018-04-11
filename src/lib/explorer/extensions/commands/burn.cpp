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

#include <metaverse/explorer/json_helper.hpp>
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/burn.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

void burn::set_defaults_from_config (po::variables_map& variables)
{
    // if --symbol="" is specified, throw an exception.
    const auto& symbol = variables["symbol"];
    if (!symbol.empty() && symbol.as<std::string>().empty()) {
        throw asset_symbol_length_exception{"asset symbol can not be empty."};
    }
}

console_result burn::invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    if (!blockchain.is_valid_address(argument_.address))
        throw fromaddress_invalid_exception{"invalid target address parameter!"};

    if (!argument_.amount)
        throw argument_legality_exception{"invalid amount parameter!"};

    auto blackhole_address = wallet::payment_address::blackhole_address;

    if (!option_.symbol.empty()) {
        // check asset symbol
        blockchain.uppercase_symbol(option_.symbol);
        if (option_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
            throw asset_symbol_length_exception{"asset symbol length must be less than 64."};

        // receiver
        std::vector<receiver_record> receiver{
            {blackhole_address, option_.symbol, 0, argument_.amount, utxo_attach_type::asset_transfer, attachment()}
        };

        auto send_helper = sending_asset(*this, blockchain, std::move(auth_.name), std::move(auth_.auth),
                std::move(argument_.address), std::move(option_.symbol), std::move(receiver), argument_.fee);

        send_helper.exec();

        // json output
        const auto& tx = send_helper.get_transaction();
        jv_output =  config::json_helper(get_api_version()).prop_tree(tx, true);

    } else {
        // receiver
        std::vector<receiver_record> receiver{
            {blackhole_address, "", argument_.amount, 0, utxo_attach_type::etp, attachment()}
        };

        auto send_helper = sending_etp(*this, blockchain, std::move(auth_.name), std::move(auth_.auth),
                std::move(argument_.address), std::move(receiver), argument_.fee);

        send_helper.exec();

        // json output
        const auto& tx = send_helper.get_transaction();
        jv_output =  config::json_helper(get_api_version()).prop_tree(tx, true);
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

