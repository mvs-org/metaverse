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
#include <metaverse/explorer/extensions/commands/lock.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


console_result lock::invoke(Json::Value& jv_output,
                            libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    bool is_asset = !is_default_invalid_asset_symbol(option_.asset_symbol);
    if (is_asset) {
        blockchain.uppercase_symbol(option_.asset_symbol);
        check_asset_symbol(option_.asset_symbol);
    }

    chain::attachment attach;
    std::string to_address = get_address_from_did(argument_.to, blockchain);
    std::string change_address = get_address(option_.change, blockchain);
    attach.set_to_did(argument_.to);

    bc::wallet::payment_address addr(to_address);
    if (addr.version() == bc::wallet::payment_address::mainnet_p2sh) { // for multisig address
        throw argument_legality_exception{"script address parameter not allowed!"};
    }

    auto sp_account_address = blockchain.get_account_address(auth_.name, to_address);
    if (!sp_account_address) {
        throw address_dismatch_account_exception{"target address does not match account. " + to_address};
    }

    if (argument_.amount <= 0) {
        throw argument_legality_exception("invalid amount parameter!");
    }

    if ((argument_.sequence & bc::relative_locktime_disabled) ||
        (argument_.sequence & bc::relative_locktime_mask) == 0) {
        throw argument_legality_exception(
            "invalid sequence parameter!" + std::to_string(argument_.sequence));
    }

    // receiver
    std::vector<receiver_record> receiver{
        {
            to_address,
            (is_asset ? option_.asset_symbol : std::string("")),
            (is_asset ? 0 : argument_.amount),
            (is_asset ? argument_.amount : 0),
            (is_asset ? utxo_attach_type::asset_transfer : utxo_attach_type::etp),
            attach,
            true
        }
    };

    if (!option_.memo.empty()) {
        check_message(option_.memo);

        receiver.push_back({
            to_address, "", 0, 0, utxo_attach_type::message,
            chain::attachment(0, 0, chain::blockchain_message(option_.memo))
        });
    }

    lock_sending send_helper(*this, blockchain,
                             std::move(auth_.name), std::move(auth_.auth),
                             "", std::move(receiver),
                             std::move(change_address),
                             option_.fee, argument_.sequence);
    send_helper.exec();

    // json output
    auto tx = send_helper.get_transaction();
    jv_output =  config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

