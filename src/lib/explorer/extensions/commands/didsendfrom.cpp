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
#include <metaverse/explorer/extensions/commands/didsendfrom.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

console_result didsendfrom::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    std::string fromaddress = "";
    std::string toaddress = "";
    attachment attach;

    //support address as well as did
    if (blockchain.is_valid_address(argument_.fromdid)) {
        fromaddress = argument_.fromdid;
    }
    else {
        if (argument_.fromdid.length() > DID_DETAIL_SYMBOL_FIX_SIZE) {
            throw did_symbol_length_exception{
                "fromdid " + argument_.fromdid + " length must be less than 64."};
        }

        if (!blockchain.is_did_exist(argument_.fromdid)) {
            throw did_symbol_notfound_exception{
                "fromdid " + argument_.fromdid + " is not exist on the blockchain."};
        }

        auto diddetail = blockchain.get_registered_did(argument_.fromdid);
        fromaddress = diddetail->get_address();
        attach.set_from_did(argument_.fromdid);
        attach.set_version(DID_ATTACH_VERIFY_VERSION);
    }

    //support address as well as did
    if (blockchain.is_valid_address(argument_.todid)) {
        toaddress = argument_.todid;
    }
    else {
        toaddress = get_address_from_did(argument_.todid,blockchain);
        attach.set_to_did(argument_.todid);
        attach.set_version(DID_ATTACH_VERIFY_VERSION);
    }

    // receiver
    std::vector<receiver_record> receiver{
        {toaddress, "", argument_.amount, 0, utxo_attach_type::etp, attach}
    };

    if (!argument_.memo.empty())
        receiver.push_back({toaddress, "", 0, 0, utxo_attach_type::message,
            attachment(0, 0, blockchain_message(argument_.memo))});

    auto send_helper = sending_etp(*this, blockchain, std::move(auth_.name), std::move(auth_.auth),
            std::move(fromaddress), std::move(receiver), argument_.fee);

    send_helper.exec();

    // json output
    auto tx = send_helper.get_transaction();
     jv_output =  config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

