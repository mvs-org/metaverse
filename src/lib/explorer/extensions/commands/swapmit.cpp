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

#include <metaverse/explorer/json_helper.hpp>
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/swapmit.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>


namespace libbitcoin {
namespace explorer {
namespace commands {

console_result swapmit::invoke(Json::Value& jv_output,
                               libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    // check foreign_addr
    if (argument_.foreign_addr.empty() || argument_.foreign_addr.size() >= 200) {
        throw argument_size_invalid_exception{"foreign address length out of bounds."};
    }

    // check ETH address
    if (!is_ETH_Address(argument_.foreign_addr)) {
        throw argument_legality_exception{argument_.foreign_addr + " is not a valid ETH address."};
    }

    // check symbol
    check_mit_symbol(argument_.symbol);

    // check to did
    auto to_did = argument_.to;
    auto to_address = get_address_from_did(to_did, blockchain);
    if (!blockchain.is_valid_address(to_address)) {
        throw toaddress_invalid_exception("Invalid did parameter! " + to_did);
    }

    // get mit
    auto mits = blockchain.get_account_mits(auth_.name, argument_.symbol);
    if (mits->size() == 0) {
        throw asset_lack_exception("Not enough asset '" + argument_.symbol +  "'");
    }

    // multisig support
    auto& mit = *(mits->begin());
    std::string from_address(mit.get_address());
    bool is_multisig_address = blockchain.is_script_address(from_address);

    chain::account_multisig acc_multisig;
    if (is_multisig_address) {
        auto multisig_vec = acc->get_multisig(from_address);
        if (!multisig_vec || multisig_vec->empty()) {
            throw multisig_notfound_exception("From address multisig record not found.");
        }

        acc_multisig = *(multisig_vec->begin());
    }

    // swap fee
    const std::string&& swapfee_address = bc::get_developer_community_address(
            blockchain.chain_settings().use_testnet_rules);

    if (option_.swapfee < DEFAULT_SWAP_FEE) {
        throw argument_legality_exception{"invalid swapfee parameter! must >= 1 ETP"};
    }

    // construct receivers
    std::vector<receiver_record> receiver{
        {
            to_address, argument_.symbol, 0, 0, 0,
            utxo_attach_type::asset_mit_transfer, chain::attachment("", to_did)
        },

        {
            swapfee_address, "", option_.swapfee, 0,
            utxo_attach_type::etp, chain::attachment()
        }
    };

    // attach memo
    if (!option_.memo.empty()) {
        check_message(option_.memo);

        receiver.push_back({
            to_address, "", 0, 0, utxo_attach_type::message,
            chain::attachment(0, 0, chain::blockchain_message(option_.memo))
        });
    }

    // attach swap info
    std::string message("{\"type\":\"ETH\",\"address\":\"" + argument_.foreign_addr + "\"}");
    receiver.push_back({
        to_address, "", 0, 0, utxo_attach_type::message,
        chain::attachment(0, 0, chain::blockchain_message(message))
    });

    auto helper = transferring_mit(
                      *this, blockchain,
                      std::move(auth_.name), std::move(auth_.auth),
                      is_multisig_address ? std::move(from_address) : "",
                      std::move(argument_.symbol),
                      std::move(receiver), option_.fee,
                      std::move(acc_multisig));

    helper.exec();

    // json output
    auto tx = helper.get_transaction();
    if (is_multisig_address) {
        jv_output = config::json_helper(get_api_version()).prop_list_of_rawtx(tx, false, true);
    }
    else {
        jv_output = config::json_helper(get_api_version()).prop_tree(tx, true);
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

