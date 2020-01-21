/**
 * Copyright (c) 2016-2020 mvs developers
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
#include <metaverse/explorer/extensions/commands/registerdid.hpp>
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

console_result registerdid::invoke(Json::Value &jv_output,
                                   libbitcoin::server::server_node &node)
{
    auto &blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    // check did symbol
    check_did_symbol(argument_.symbol, true);

    if (blockchain.is_valid_address(argument_.symbol)) {
        throw address_invalid_exception{"symbol cannot be an address!"};
    }

    // check fee
    if (argument_.fee < bc::min_fee_to_register_did) {
        throw did_register_poundage_exception{
            "register did: fee less than "
            + std::to_string(bc::min_fee_to_register_did) + " that's "
            + std::to_string(bc::min_fee_to_register_did / 100000000) + " ETPs"};
    }

    if (argument_.percentage < bc::min_fee_percentage_to_miner || argument_.percentage > 100) {
        throw did_register_poundage_exception{
            "register did minimum percentage of fee to miner less than "
            + std::to_string(bc::min_fee_percentage_to_miner)
            + " or greater than 100."};
    }

    // check address
    if (!blockchain.is_valid_address(argument_.address)) {
        throw address_invalid_exception{"invalid address parameter!"};
    }

    if (!blockchain.get_account_address(auth_.name, argument_.address)) {
        throw address_dismatch_account_exception{
            "address " + argument_.address + " is not owned by " + auth_.name};
    }

    // fail if did is already in blockchain
    if (blockchain.is_did_exist(argument_.symbol))
        throw did_symbol_existed_exception{"did symbol already exists on the blockchain"};

    // fail if address is already binded with did in blockchain
    if (blockchain.is_address_registered_did(argument_.address))
        throw did_symbol_existed_exception{"address is already binded with some did on the blockchain"};

    account_multisig acc_multisig;
    bool is_multisig_address = blockchain.is_script_address(argument_.address);
    if (is_multisig_address) {
        auto multisig_vec = acc->get_multisig(argument_.address);
        if (!multisig_vec || multisig_vec->empty()) {
            throw multisig_notfound_exception{"multisig of address does not found."};
        }

        acc_multisig = *(multisig_vec->begin());
    }

    // receiver
    std::vector<receiver_record> receiver{
        {argument_.address, argument_.symbol, 0, 0, utxo_attach_type::did_register, attachment()}};

    auto register_helper = registering_did(
                               *this, blockchain, std::move(auth_.name), std::move(auth_.auth),
                               std::move(argument_.address), std::move(argument_.symbol),
                               std::move(receiver), argument_.fee, argument_.percentage,
                               std::move(acc_multisig));

    register_helper.exec();

    // output json
    auto && tx = register_helper.get_transaction();
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
