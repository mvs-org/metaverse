/**
 * Copyright (c) 2016-2017 mvs developers 
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
#include <metaverse/explorer/extensions/commands/getnewmultisig.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

console_result getnewmultisig::invoke(Json::Value& jv_output,
     libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    // parameter account name check
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    //auto acc_multisig = acc->get_multisig();
    account_multisig acc_multisig;

    if (option_.public_keys.empty())
        throw multisig_cosigne_exception{ "multisig cosigner public key needed." };
    // parameter check
    if (option_.m < 1)
        throw signature_amount_exception{ "signature number less than 1." };
    if (!option_.n || option_.n > 20)
        throw pubkey_amount_exception{ "public key number bigger than 20." };
    if (option_.m > option_.n)
        throw signature_amount_exception{ "signature number bigger than public key number." };

    // add self public key into key vector
    auto pubkey = option_.self_publickey;
    if (std::find(option_.public_keys.begin(), option_.public_keys.end(), pubkey) == option_.public_keys.end()) // not found
        option_.public_keys.push_back(pubkey);
    if (option_.n != option_.public_keys.size())
        throw pubkey_amount_exception{ "public key number not match with n." };

    acc_multisig.set_hd_index(0);
    acc_multisig.set_m(option_.m);
    acc_multisig.set_n(option_.n);
    acc_multisig.set_pubkey(pubkey);
    acc_multisig.set_cosigner_pubkeys(std::move(option_.public_keys));
    acc_multisig.set_description(option_.description);

    if (acc->get_multisig(acc_multisig))
        throw multisig_exist_exception{ "multisig already exists." };

    acc_multisig.set_index(acc->get_multisig_vec().size() + 1);

    // change account type
    acc->set_type(account_type::multisignature);


    // store address
    auto addr = std::make_shared<bc::chain::account_address>();
    addr->set_name(auth_.name);

    // get private key according public key
    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if (!pvaddr)
        throw address_list_nullptr_exception{ "nullptr for address list" };

    std::string prv_key;
    auto found = false;
    for (auto& each : *pvaddr) {

        prv_key = each.get_prv_key(auth_.auth);
        auto&& target_pub_key = ec_to_xxx_impl("ec-to-public", prv_key);
        if (target_pub_key == pubkey) {
            found = true;
            break;
        }

    }

    if (!found) {
        throw pubkey_dismatch_exception{ pubkey + " not belongs to this account" };
    }

    addr->set_prv_key(prv_key, auth_.auth);

    // multisig address
    auto multisig_script = acc_multisig.get_multisig_script();
    chain::script script_inst;
    script_inst.from_string(multisig_script);
    if (script_pattern::pay_multisig != script_inst.pattern())
        throw multisig_script_exception{ std::string("invalid multisig script : ") + multisig_script };
    payment_address address(script_inst, 5);

    addr->set_address(address.encoded());
    //addr->set_status(1); // 1 -- enable address
    addr->set_status(account_address_status::multisig_addr);

    auto addr_str = address.encoded();
    acc_multisig.set_address(addr_str);
    acc->set_multisig(acc_multisig);

    blockchain.store_account(acc);
    blockchain.store_account_address(addr);

    Json::Value root, pubkeys;

    if (get_api_version() == 1) {
        root["index"] += acc_multisig.get_index();
        root["m"] += acc_multisig.get_m();
        root["n"] += acc_multisig.get_n();
    } else {
        root["index"] = acc_multisig.get_index();
        root["m"] = acc_multisig.get_m();
        root["n"] = acc_multisig.get_n();
    }
    root["self-publickey"] = acc_multisig.get_pubkey();
    root["description"] = acc_multisig.get_description();

    for (auto& each : acc_multisig.get_cosigner_pubkeys()) {
        pubkeys.append(each);
    }
    root["public-keys"] = pubkeys;
    root["multisig-script"] = multisig_script;
    root["address"] = acc_multisig.get_address();


    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

