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
#include <metaverse/explorer/extensions/commands/getnewaccount.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/commands/offline_commands_impl.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ getnewaccount *************************/

//76066276 is HD private key version
const uint64_t prefixes = wallet::hd_private::to_prefixes(76066276, 0);

console_result getnewaccount::invoke(Json::Value& jv_output,
    bc::server::server_node& node)
{
    if (auth_.name.empty() && auth_.auth.empty()) {
        return create_address(jv_output, node);
    }

    return create_account(jv_output, node);
}

console_result getnewaccount::create_account(Json::Value& jv_output,
    bc::server::server_node& node)
{
#ifdef NDEBUG
    if (auth_.name.length() > 128 || auth_.name.length() < 3 ||
        auth_.auth.length() > 128 || auth_.auth.length() < 6) {
        throw argument_legality_exception{"name length in [3, 128], password length in [6, 128]"};
    }
#endif

    auto& blockchain = node.chain_impl();
    if (blockchain.is_account_exist(auth_.name)) {
        throw account_existed_exception{"account already exist"};
    }

    auto acc = std::make_shared<bc::chain::account>();
    acc->set_name(auth_.name);
    acc->set_passwd(auth_.auth);

    bc::explorer::config::language opt_language(option_.language);
    auto&& seed = get_seed();
    auto&& words_list = get_mnemonic_new(opt_language , seed);
    auto&& words = bc::join(words_list);

    acc->set_mnemonic(words, auth_.auth);

    // flush to db
    auto ret = blockchain.store_account(acc);

    // get 1 new sub-address by default
    std::stringstream sout("");
    Json::Value jv_temp;
    const char* cmds2[]{"getnewaddress", auth_.name.c_str(), auth_.auth.c_str()};

    if (dispatch_command(3, cmds2, jv_temp, node, get_api_version()) != console_result::okay) {
        throw address_generate_exception(sout.str());
    }

    if (get_api_version() == 1) {
        jv_output["mnemonic"] = words;
        jv_output["default-address"] = jv_temp;
    }
    else if (get_api_version() == 2) {
        jv_output["mnemonic"] = words;
        jv_output["default-address"] = jv_temp["addresses"][0].asString();
    }
    else {
        config::json_helper::account_info acc(auth_.name, words, jv_temp);
        jv_output = config::json_helper(get_api_version()).prop_list(acc);
    }

    return console_result::okay;
}

console_result getnewaccount::create_address(Json::Value& jv_output,
    bc::server::server_node& node)
{
    auto& blockchain = node.chain_impl();

    // mainnet payment address version
    auto payment_version = wallet::payment_address::mainnet_p2kh;
    if (blockchain.chain_settings().use_testnet_rules) {
        // testnet payment address version
        payment_version = 127;
    }

    // create mnemonic words
    bc::explorer::config::language opt_language(option_.language);
    auto words = get_mnemonic_new(opt_language, get_seed());
    std::string mnemonic = bc::join(words);

    // create master hd private
    const auto seed = wallet::decode_mnemonic(words);
    bc::config::base16 bs(seed);
    const data_chunk& ds = static_cast<const data_chunk&>(bs);
    const wallet::hd_private master_hd_private(ds, prefixes);

    // create derive hd private at index 0
    const auto derive_hd_private = master_hd_private.derive_private(0);
    std::string hk = derive_hd_private.encoded();
    const auto derive_private_key = wallet::hd_private(hk, prefixes);

    // get public key and payment address
    ec_secret secret = derive_private_key.secret();
    wallet::ec_private ec_prv(secret);
    ec_compressed point;
    bc::secret_to_public(point, secret);
    wallet::ec_public ec_pub(point, true);
    wallet::payment_address pay_address(ec_pub, payment_version);

    // encode
    std::string prv_key = encode_base16(secret);
    std::string prv_key_wif = ec_prv.encoded();
    std::string pub_key = ec_pub.encoded();
    std::string addr = pay_address.encoded();

    // output
    jv_output["mnemonic"] = mnemonic;
    jv_output["private_key"] = prv_key;
    jv_output["public_key"] = pub_key;
    jv_output["wif"] = prv_key_wif;
    jv_output["address"] = addr;

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

