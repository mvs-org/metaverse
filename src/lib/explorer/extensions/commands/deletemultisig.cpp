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
#include <metaverse/explorer/extensions/commands/deletemultisig.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

using namespace bc::explorer::config;

console_result deletemultisig::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    if (!blockchain.is_valid_address(option_.address)) {
        throw fromaddress_invalid_exception("invalid address! "  + option_.address);
    }

    auto addr = bc::wallet::payment_address(option_.address);
    if (addr.version() != bc::wallet::payment_address::mainnet_p2sh) {
        throw fromaddress_invalid_exception("address " + option_.address + " is not a script address.");
    }

    // get multisig
    auto multisig_vec = acc->get_multisig(option_.address);
    if (!multisig_vec || multisig_vec->empty()) {
        throw multisig_notfound_exception(" multisig of address " + option_.address + " is not found.");
    }

    // remove multisig
    for(auto& acc_multisig : *multisig_vec) {
        acc->remove_multisig(acc_multisig);
    }

    // change account type
    if (acc->get_multisig_vec().empty()) {
        acc->set_type(account_type::common);
    }

    // flush to db
    blockchain.store_account(acc);

    // delete account address
    auto vaddr = blockchain.get_account_addresses(auth_.name);
    if (!vaddr) {
        throw address_list_empty_exception{"empty address list for this account"};
    }

    blockchain.delete_account_address(auth_.name);
    for (auto it = vaddr->begin(); it != vaddr->end();) {
        if (it->get_address() == option_.address) {
            it = vaddr->erase(it);
        }
        else {
            ++it;
        }
    }

    // restore address
    for (auto& each : *vaddr) {
        auto addr = std::make_shared<bc::chain::account_address>(each);
        blockchain.store_account_address(addr);
    }

    // output json
    Json::Value nodes;
    auto helper = config::json_helper(get_api_version());
    for(auto& acc_multisig : *multisig_vec) {
        Json::Value node = helper.prop_list(acc_multisig);
        nodes.append(node);
    }

    if (get_api_version() == 1 && nodes.isNull()) { // compatible for v1
        jv_output["multisig"] = "";
    }
    else {
        jv_output["multisig"] = nodes;
    }

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

