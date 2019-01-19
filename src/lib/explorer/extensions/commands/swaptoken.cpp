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
#include <metaverse/explorer/extensions/commands/swaptoken.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>
#include <metaverse/consensus/libdevcore/SHA3.h>
#include <boost/algorithm/string.hpp>
#include <regex>

namespace libbitcoin {
namespace explorer {
namespace commands {


console_result swaptoken::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(argument_.symbol);

    // check message
    if (argument_.foreign_addr.empty() || argument_.foreign_addr.size() >= 200) {
        throw argument_size_invalid_exception{"foreign address length out of bounds."};
    }

    // check ETH address
    if (!is_ETH_Address(argument_.foreign_addr)) {
        throw argument_legality_exception{argument_.foreign_addr + " is not a valid ETH address."};
    }

    // check asset symbol
    check_asset_symbol(argument_.symbol);

    chain::attachment attach_asset, attach_fee;
    const std::string&& to_address = get_address(argument_.to, attach_asset, false, blockchain);
    const std::string&& swapfee_address = bc::get_developer_community_address(
        blockchain.chain_settings().use_testnet_rules);

    std::string from_address("");
    if (!option_.from.empty()) {
        from_address = get_address(option_.from, attach_asset, true, blockchain);
                       get_address(option_.from, attach_fee, true, blockchain);
    }

    std::string change_address = get_address(option_.change, blockchain);

    if (!argument_.amount) {
        throw argument_legality_exception{"invalid amount parameter!"};
    }
    if (option_.swapfee < DEFAULT_SWAP_FEE) {
        throw argument_legality_exception{"invalid swapfee parameter! must >= 1 ETP"};
    }

    std::vector<receiver_record> receiver{
        {to_address, argument_.symbol, 0, argument_.amount, utxo_attach_type::asset_transfer, attach_asset},
        {swapfee_address, "", option_.swapfee, 0, utxo_attach_type::etp, attach_fee},
    };

    if (!option_.memo.empty()) {
        check_message(option_.memo);

        receiver.push_back({to_address, "", 0, 0, utxo_attach_type::message,
            chain::attachment(0, 0, chain::blockchain_message(option_.memo))});
    }

    std::string message("{\"type\":\"ETH\",\"address\":\""+ argument_.foreign_addr + "\"}");

    auto send_helper = sending_asset(
        *this, blockchain,
         std::move(auth_.name), std::move(auth_.auth),
         std::move(from_address), std::move(argument_.symbol),
         "",
         std::move(receiver), option_.fee,
         std::move(message), std::move(change_address));

    send_helper.exec();

    // json output
    auto tx = send_helper.get_transaction();
    jv_output =  config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

