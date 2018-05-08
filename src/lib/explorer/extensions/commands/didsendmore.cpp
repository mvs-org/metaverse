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
#include <metaverse/explorer/extensions/commands/didsendmore.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


console_result didsendmore::invoke (Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    auto&& changesymbol = argument_.mychange_address;
    if (!changesymbol.empty() && !blockchain.is_valid_address(changesymbol))
    {
        if (changesymbol.length() > DID_DETAIL_SYMBOL_FIX_SIZE) {
            throw did_symbol_length_exception{
                "mychange did symbol [" + changesymbol + "] length must be less than 64."};
        }

        if (!blockchain.get_issued_did(changesymbol)) {
            throw did_symbol_notfound_exception{
                "mychange did symbol [" + changesymbol + "] is not exist in blockchain"};
        }
    }

    // receiver
    std::vector<receiver_record> receiver;

    for (auto& each : argument_.receivers) {
        attachment attach;
        std::string address;

        colon_delimited2_item<std::string, uint64_t> item(each);
        auto && addressordid = item.first();

        //did check
        if(blockchain.is_valid_address(addressordid)) {
            address = addressordid;
        }
        else {
            if (addressordid.length() > DID_DETAIL_SYMBOL_FIX_SIZE) {
                throw did_symbol_length_exception(
                    "receiver did symbol [" + addressordid + "] length must be less than 64.");
            }

            auto diddetail = blockchain.get_issued_did(addressordid);
            if (!diddetail) {
                throw did_symbol_notfound_exception(
                    "receiver did symbol [" + addressordid + "] is not exist in blockchain");
            }

            attach.set_to_did(addressordid);
            attach.set_version(DID_ATTACH_VERIFY_VERSION);

            address = diddetail->get_address();
        }

        if (!item.second())
            throw argument_legality_exception("invalid amount parameter!" + each);

        receiver.push_back({address,"", item.second(), 0, utxo_attach_type::etp, attach});
    }

    auto send_helper = sending_etp_more(*this, blockchain, std::move(auth_.name), std::move(auth_.auth),
            "", std::move(receiver), std::move(changesymbol), argument_.fee);

    send_helper.exec();

    // json output
    auto tx = send_helper.get_transaction();
     jv_output =  config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

