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
#include <metaverse/explorer/extensions/commands/getnewmultisig.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

console_result getnewmultisig::invoke(
    Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();

    // check auth
    auto account = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    auto& pubkey_vec = option_.public_keys;
    if (pubkey_vec.empty()) {
        throw multisig_cosigne_exception{ "multisig cosigner public key needed." };
    }

    std::set<std::string> unique_keys(pubkey_vec.begin(), pubkey_vec.end());
    if (unique_keys.size() != pubkey_vec.size()) {
        throw multisig_cosigne_exception{ "multisig cosigner public key has duplicated items." };
    }

    // check m & n
    if (option_.m < 1) {
        throw signature_amount_exception{ "signature number less than 1." };
    }
    if (option_.n < 1 || option_.n > 20) {
        throw pubkey_amount_exception{
            "public key number " + std::to_string(option_.n) + " less than 1 or bigger than 20."};
    }
    if (option_.m > option_.n) {
        throw signature_amount_exception{
            "signature number " + std::to_string(option_.m)
            + " bigger than public key number " + std::to_string(option_.n) };
    }

    // check self public key
    auto self_pubkey = option_.self_publickey;
    if (self_pubkey.empty()) {
        throw pubkey_notfound_exception{ "self pubkey key not found!" };
    }

    // if self public key not in public keys then add it.
    auto iter = std::find(pubkey_vec.begin(), pubkey_vec.end(), self_pubkey);
    if (iter == pubkey_vec.end()) {
        pubkey_vec.push_back(self_pubkey);
    }

    // check public key size
    if (option_.n != pubkey_vec.size()) {
        throw pubkey_amount_exception{ "public key number not match with n." };
    }

    // get private key according public key
    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if (!pvaddr) {
        throw address_list_nullptr_exception{ "nullptr for address list" };
    }

    std::string self_prvkey;
    auto found = false;
    for (auto& each : *pvaddr) {
        self_prvkey = each.get_prv_key(auth_.auth);
        auto&& target_pub_key = ec_to_xxx_impl("ec-to-public", self_prvkey);
        if (target_pub_key == self_pubkey) {
            found = true;
            break;
        }
    }

    if (!found) {
        throw pubkey_dismatch_exception{ self_pubkey + " not belongs to this account" };
    }

    // generate multisig account
    account_multisig acc_multisig;
    acc_multisig.set_hd_index(0);
    acc_multisig.set_m(option_.m);
    acc_multisig.set_n(option_.n);
    acc_multisig.set_pub_key(self_pubkey);
    acc_multisig.set_cosigner_pubkeys(std::move(pubkey_vec));
    acc_multisig.set_description(option_.description);

    // check same multisig account not exists
    if (account->is_multisig_exist(acc_multisig))
        throw multisig_exist_exception{ "multisig already exists." };

    // update index
    acc_multisig.set_index(account->get_multisig_vec().size() + 1);

    // change account type
    account->set_type(account_type::multisignature);

    // create account address
    auto account_address = std::make_shared<bc::chain::account_address>();
    account_address->set_name(auth_.name);
    account_address->set_prv_key(self_prvkey, auth_.auth);

    // create payment script and address
    auto multisig_script = acc_multisig.get_multisig_script();
    chain::script payment_script;
    payment_script.from_string(multisig_script);
    if (script_pattern::pay_multisig != payment_script.pattern())
        throw multisig_script_exception{ std::string("invalid multisig script : ") + multisig_script };

    payment_address address(payment_script, payment_address::mainnet_p2sh);
    auto hash_address = address.encoded();

    // update account and multisig account
    // account_address->set_status(1); // 1 -- enable address
    account_address->set_address(hash_address);
    account_address->set_status(account_address_status::multisig_addr);

    acc_multisig.set_address(hash_address);
    account->set_multisig(acc_multisig);

    // store them
    blockchain.store_account(account);
    blockchain.store_account_address(account_address);

    // output json
    jv_output = config::json_helper(get_api_version()).prop_list(acc_multisig);
    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

