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
#include <metaverse/explorer/extensions/commands/didmodifyaddress.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

console_result didmodifyaddress::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    // check did symbol
    auto did = argument_.symbol;

    check_did_symbol(did);

    // check did exsits
    auto did_detail = blockchain.get_issued_did(did);
    if (!did_detail) {
        throw did_symbol_notfound_exception{"Did '" + did + "' does not exist on the blockchain"};
    }

    auto from_address = did_detail->get_address();

    // check did is owned by the account
    if (!blockchain.get_account_address(auth_.name, from_address)) {
        throw did_symbol_notowned_exception{
            "Did '" + did + "' is not owned by " + auth_.name};
    }

    // check to address is valid
    if (!blockchain.is_valid_address(argument_.to))
        throw toaddress_invalid_exception{"Invalid target address parameter!"};

    // check to address is owned by the account
    if (!blockchain.get_account_address(auth_.name, argument_.to)) {
        throw address_dismatch_account_exception{"Target address is not owned by account. " + argument_.to};
    }

     // fail if address is already binded with did in blockchain
    if (blockchain.is_address_issued_did(argument_.to)) {
        throw did_symbol_existed_exception{"Target address is already binded with some did on the blockchain"};
    }

    // receiver
    std::vector<receiver_record> receiver{
        {argument_.to, argument_.symbol, 0, 0, utxo_attach_type::did_transfer, attachment()}
    };

    auto toaddr = bc::wallet::payment_address(argument_.to);
    auto addr = bc::wallet::payment_address(from_address);

    if( toaddr.version() == bc::wallet::payment_address::mainnet_p2sh
    && addr.version() == bc::wallet::payment_address::mainnet_p2sh)
        throw did_multisig_address_exception{"did cannot modify multi-signature address to multi-signature address"};


    if (addr.version() == bc::wallet::payment_address::mainnet_p2sh
    || toaddr.version() == bc::wallet::payment_address::mainnet_p2sh) // for multisig address
    {

        auto findmultisig = [&acc](account_multisig& acc_multisig, std::string address) {
            auto multisig_vec = acc->get_multisig(address);
            if (!multisig_vec || multisig_vec->empty())
                return false;

            acc_multisig = *(multisig_vec->begin());
            return true;
        };

        account_multisig acc_multisig;
        if (addr.version() == bc::wallet::payment_address::mainnet_p2sh && !findmultisig(acc_multisig, from_address))
            throw multisig_notfound_exception{"from address multisig record not found."};

        account_multisig acc_multisig_to;
        if (toaddr.version() == bc::wallet::payment_address::mainnet_p2sh && !findmultisig(acc_multisig_to, argument_.to))
            throw multisig_notfound_exception{"to address multisig record not found."};

        auto issue_helper = sending_multisig_did(*this, blockchain, std::move(auth_.name), std::move(auth_.auth),
                                                 std::move(from_address),  std::move(argument_.to),
                                                 std::move(argument_.symbol), std::move(receiver), argument_.fee,
                                                 std::move(acc_multisig), std::move(acc_multisig_to));

        issue_helper.exec();
        // json output
        auto && tx = issue_helper.get_transaction();
        std::ostringstream tx_buf;
        tx_buf << config::transaction(tx);
        jv_output = tx_buf.str();
        // TODO support restful API format
        // jv_output["raw"] = tx_buf.str();
    }
    else
    {
        auto send_helper = sending_did(*this, blockchain,
                                       std::move(auth_.name), std::move(auth_.auth),
                                       std::move(from_address), std::move(argument_.to),
                                       std::move(argument_.symbol), std::move(receiver), argument_.fee);

        send_helper.exec();

        // json output
        auto tx = send_helper.get_transaction();
        jv_output = config::json_helper(get_api_version()).prop_tree(tx, true);
    }




    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

