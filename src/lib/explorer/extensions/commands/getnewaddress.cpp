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


#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/getnewaddress.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;


/************************ getnewaddress *************************/

console_result getnewaddress::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    std::string mnemonic;
    acc->get_mnemonic(auth_.auth, mnemonic);
    if (mnemonic.empty()) { throw mnemonicwords_empty_exception("mnemonic empty"); }
    if (!option_.count) { throw address_amount_exception("invalid address number parameter"); }
    
    std::vector<std::string> words;
    words.reserve(24);

    //split mnemonic to words
    string::size_type pos1, pos2;
    std::string c{" "};
    pos2 = mnemonic.find(c);
    pos1 = 0;
    while(string::npos != pos2)
    {
        words.push_back(mnemonic.substr(pos1, pos2-pos1));

        pos1 = pos2 + c.size();
        pos2 = mnemonic.find(c, pos1);
    }
    if(pos1 != mnemonic.length())
        words.push_back(mnemonic.substr(pos1));
    //end split

    if ((words.size() % bc::wallet::mnemonic_word_multiple) != 0) {
        return console_result::failure;
    }


    uint32_t idx = 0;
    pt::ptree aroot;
    pt::ptree addresses;

    std::vector<std::shared_ptr<account_address>> account_addresses;
    account_addresses.reserve(option_.count);
    const auto seed = decode_mnemonic(words);
    libbitcoin::config::base16 bs(seed);
    const data_chunk& ds = static_cast<const data_chunk&>(bs);
    const auto prefixes = bc::wallet::hd_private::to_prefixes(76066276, 0);//76066276 is HD private key version
    const bc::wallet::hd_private private_key(ds, prefixes);
    // mainnet payment address version
    auto payment_version = 50;

    if (blockchain.chain_settings().use_testnet_rules){
         // testnetpayment address version
         payment_version = 127;
    }

    for ( idx = 0; idx < option_.count; idx++ ) {

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

        // Serialize to the original compression state.
        auto ep =  ec_public(point, true);

        payment_address pa(ep, payment_version);

        addr->set_address(pa.encoded());
        addr->set_status(1); // 1 -- enable address

        acc->increase_hd_index();
        addr->set_hd_index(acc->get_hd_index());
        account_addresses.push_back(addr);

        // write to output json
        pt::ptree address;
        address.put("", addr->get_address());
        addresses.push_back(std::make_pair("", address));
        if(option_.count == 1)
            output<<addr->get_address();
    }

    blockchain.safe_store_account(*acc, account_addresses);

    aroot.add_child("addresses", addresses);

    if(option_.count != 1)
        pt::write_json(output, aroot);
    
    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

