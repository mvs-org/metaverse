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


#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/getnewaddress.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/bitcoin/wallet/vrf_private.hpp>
#include <metaverse/macros_define.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ getnewaddress *************************/

console_result getnewaddress::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    std::string mnemonic;
    acc->get_mnemonic(auth_.auth, mnemonic);
    if (mnemonic.empty()) {
        throw mnemonicwords_empty_exception("mnemonic empty");
    }
    if (!option_.count || (option_.count & 0xfff00000)) {
        throw address_amount_exception("invalid address number parameter");
    }

    //split mnemonic to vector words
    auto&& words = bc::split(mnemonic, " ", true); // with trim

    if ((words.size() % bc::wallet::mnemonic_word_multiple) != 0) {
        throw mnemonicwords_amount_exception{"invalid size of backup words."};
    }

    Json::Value addresses;

    std::vector<std::shared_ptr<account_address>> account_addresses;
    account_addresses.reserve(option_.count);
    const auto seed = wallet::decode_mnemonic(words);
    libbitcoin::config::base16 bs(seed);
    const data_chunk& ds = static_cast<const data_chunk&>(bs);
    const auto prefixes = bc::wallet::hd_private::to_prefixes(76066276, 0);//76066276 is HD private key version
    const bc::wallet::hd_private private_key(ds, prefixes);

    // mainnet payment address version
    auto payment_version = 50;
    if (blockchain.chain_settings().use_testnet_rules) {
        // testnetpayment address version
        payment_version = 127;
    }

    for (uint32_t idx = 0; idx < option_.count; idx++ ) {

        auto addr = std::make_shared<bc::chain::account_address>();
        addr->set_name(auth_.name);

        const auto child_private_key = private_key.derive_private(acc->get_hd_index());
        auto hk = child_private_key.encoded();

        // Create the private key from hd_key and the public version.
        const auto derive_private_key = bc::wallet::hd_private(hk, prefixes);
        auto pk = encode_base16(derive_private_key.secret());

        addr->set_prv_key(pk.c_str(), auth_.auth);
        // not store public key now
        ec_compressed point;
        libbitcoin::secret_to_public(point, derive_private_key.secret());


#ifdef PRIVATE_CHAIN
        wallet::vrf_private vrf_(derive_private_key.secret());

        log::info("VRF") << "pri:" << vrf_.encoded()
            << ", pub:" << encode_base16(vrf_.to_public());

        data_chunk data{1,2,3,4,5,6,7,8,9,10};
        wallet::vrf_proof proof;
        wallet::vrf_hash result;
        if(!vrf_.prove(proof, data) 
            || !wallet::vrf_private::proof_to_hash(result, proof)){
            log::error("Wallet")<<"generate prove failed!";
        }

        log::info("VRF") << "generate prove finish, msg:" << encode_base16(data) 
            << ", proof:" << encode_base16(proof)
            << ", result:" << encode_base16(result);

        wallet::vrf_hash result1;
        if(!wallet::vrf_private::verify(result1, proof, vrf_.to_public(), data)
        || result != result1){
            log::error("VRF")<< "verify failed!"
                << ", result:"<< encode_base16(result)
                << ", result1:"<< encode_base16(result1);
        }
        
        log::error("Wallet")<<"ecvrf_verify success!"
            << ", result1:" << encode_base16(result1);
#endif

        // Serialize to the original compression state.
        auto ep =  wallet::ec_public(point, true);

        wallet::payment_address pa(ep, payment_version);

        addr->set_address(pa.encoded());
        addr->set_status(1); // 1 -- enable address

        acc->increase_hd_index();
        addr->set_hd_index(acc->get_hd_index());
        account_addresses.push_back(addr);

        addresses.append(addr->get_address());
    }

    blockchain.safe_store_account(*acc, account_addresses);

    // write to output json
    if (get_api_version() == 1 && option_.count == 1) {
        jv_output = addresses[0];
    }
    else if (get_api_version() <= 2) {
        jv_output["addresses"] = addresses;
    }
    else {
        if(addresses.isNull())
            addresses.resize(0);
        jv_output = addresses;
    }

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

