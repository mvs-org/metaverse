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
#include <metaverse/explorer/extensions/commands/issuedid.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin
{
namespace explorer
{
namespace commands
{

console_result issuedid::invoke(Json::Value &jv_output,
                                libbitcoin::server::server_node &node)
{
    auto &blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    // check did symbol
    check_did_symbol(argument_.symbol, true);

    if (blockchain.is_valid_address(argument_.symbol))
        throw address_invalid_exception{"symbol cannot be an address!"};

    if (argument_.fee < 100000000)
        throw did_issue_poundage_exception{"issue did fee must be at least 100000000 satoshi = 1 etp!"};
    if (!blockchain.is_valid_address(argument_.address))
        throw address_invalid_exception{"invalid address parameter!"};

    if (!blockchain.get_account_address(auth_.name, argument_.address)) {
        throw address_dismatch_account_exception{
            "address " + argument_.address + " is not owned by " + auth_.name};
    }

    // fail if did is already in blockchain
    if (blockchain.is_did_exist(argument_.symbol))
        throw did_symbol_existed_exception{"did symbol already exists on the blockchain"};

    // fail if address is already binded with did in blockchain
    if (blockchain.is_address_issued_did(argument_.address))
        throw did_symbol_existed_exception{"address is already binded with some did on the blockchain"};

    auto addr = bc::wallet::payment_address(argument_.address);

    // receiver
    std::vector<receiver_record> receiver{
        {argument_.address, argument_.symbol, 0, 0, utxo_attach_type::did_issue, attachment()}};

    // for multisig address
    if (addr.version() == bc::wallet::payment_address::mainnet_p2sh) {
        auto multisig_vec = acc->get_multisig(argument_.address);
        if (!multisig_vec || multisig_vec->empty()) {
            throw multisig_notfound_exception{"multisig of address does not found."};
        }

        account_multisig acc_multisig = *(multisig_vec->begin());
        auto issue_helper = issuing_multisig_did(
                                *this, blockchain, std::move(auth_.name), std::move(auth_.auth),
                                std::move(argument_.address), std::move(argument_.symbol),
                                std::move(receiver), argument_.fee,
                                acc_multisig);
        issue_helper.exec();

        // output json
        auto && tx = issue_helper.get_transaction();
        std::ostringstream tx_buf;
        tx_buf << config::transaction(tx);
        jv_output = tx_buf.str();
        // TODO support restful API format
        // jv_output["raw"] = tx_buf.str();
    }
    else {
        auto issue_helper = issuing_did(
                                *this, blockchain, std::move(auth_.name), std::move(auth_.auth),
                                std::move(argument_.address), std::move(argument_.symbol),
                                std::move(receiver), argument_.fee);
        issue_helper.exec();

        auto &&tx = issue_helper.get_transaction();
        jv_output = config::json_helper(get_api_version()).prop_tree(tx, true);
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
