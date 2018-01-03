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
#include <metaverse/explorer/extensions/commands/signmultisigtx.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

console_result signmultisigtx::invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    // get not signed tx
    // jv_output =  "###### raw tx ######" << std::endl;
    // jv_output =  config::json_helper(get_api_version()).prop_tree(argument_.transaction, true));
    tx_type tx_ = argument_.transaction;

    // get all address of this account
    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr) 
        throw address_list_empty_exception{"empty address list for this account."};
    
    bc::chain::script ss;
    bc::chain::script redeem_script;

    auto passwd_ = auth_.auth;

    std::string multisig_script, addr_prikey;
    uint32_t index = 0;
    for(auto& each_input : tx_.inputs) {
        ss = each_input.script;
        log::trace("wdy old script=") << ss.to_string(false);
        const auto& ops = ss.operations;
        
        // 1. extract address from multisig payment script
        // zero sig1 sig2 ... encoded-multisig
        const auto& redeem_data = ops.back().data;
        
        if (redeem_data.empty())
            throw redeem_script_empty_exception{"empty redeem script."};
        
        if (!redeem_script.from_data(redeem_data, false, bc::chain::script::parse_mode::strict))
            throw redeem_script_data_exception{"error occured when parse redeem script data."};
        
        // Is the redeem script a standard pay (output) script?
        const auto redeem_script_pattern = redeem_script.pattern();
        if(redeem_script_pattern != script_pattern::pay_multisig)
            throw redeem_script_pattern_exception{"redeem script is not pay multisig pattern."};
        
        const payment_address address(redeem_script, 5);
        auto addr_str = address.encoded(); // pay address
        
        // 2. get address prikey
        account_multisig acc_multisig;
        if(!(acc->get_multisig_by_address(acc_multisig, addr_str)))
            throw multisig_notfound_exception{addr_str + " multisig record not found."};
        
        if(ops.size() >= acc_multisig.get_m() + 2) { // signed , nothing to do (2 == zero encoded-script)
            index++;
            continue;
        }

        addr_prikey = "";
        for (auto& each : *pvaddr){
            if ( addr_str == each.get_address() ) { // find address
                addr_prikey = each.get_prv_key(passwd_);
                break;
            }
        }
        if(addr_prikey.empty())
            throw prikey_notfound_exception{ addr_str + "private key not found."};
        // 3. populate unlock script
        multisig_script = acc_multisig.get_multisig_script();
        log::trace("wdy script=") << multisig_script;
        //wallet::payment_address payment("3JoocenkYHEKFunupQSgBUR5bDWioiTq5Z");
        //log::trace("wdy hash=") << libbitcoin::config::base16(payment.hash());
        // prepare sign
        explorer::config::hashtype sign_type;
        uint8_t hash_type = (signature_hash_algorithm)sign_type;
        
        bc::explorer::config::ec_private config_private_key(addr_prikey);
        const ec_secret& private_key =    config_private_key;    
        
        bc::explorer::config::script config_contract(multisig_script);
        const bc::chain::script& contract = config_contract;
        
        // gen sign
        bc::endorsement endorse;
        if (!bc::chain::script::create_endorsement(endorse, private_key,
            contract, tx_, index, hash_type))
        {
            throw tx_sign_exception{"get_input_sign sign failure"};
        }
        // insert endorse before multisig script
        auto position = ss.operations.end();
        ss.operations.insert(position - 1, {bc::chain::opcode::special, endorse});
        // rearange signature order
        bc::chain::script new_ss;
        data_chunk data;
        chain::script script_encoded;
        script_encoded.from_string(multisig_script);
        
        new_ss.operations.push_back(ss.operations.front()); // skip first "m" and last "n checkmultisig"
        for( auto pubkey_it = (script_encoded.operations.begin() + 1 ); 
            pubkey_it != (script_encoded.operations.end() - 2); ++pubkey_it) {
            for (auto it = (ss.operations.begin() + 1); it != (ss.operations.end() - 1); ++it){ // skip first "zero" and last "encoded-script"
                auto endorsement = it->data;
                const auto sighash_type = endorsement.back();
                auto distinguished = endorsement;
                distinguished.pop_back();
            
                ec_signature signature;
                auto strict = ((script_context::all_enabled & script_context::bip66_enabled) != 0); // from validate_transaction.cpp handle_previous_tx
                if (!parse_signature(signature, distinguished, strict))
                    continue;
            
                if (chain::script::check_signature(signature, sighash_type, pubkey_it->data,
                    script_encoded, tx_, index)) {
                    new_ss.operations.push_back(*it);
                    break;
                }
                
            }
        }
        new_ss.operations.push_back(ss.operations.back());
                
        // set input script of this tx
        each_input.script = new_ss;
        index++;
        log::trace("wdy new script=") << ss.to_string(false);
    }
        
    // jv_output =  config::json_helper(get_api_version()).prop_tree(tx_, true);
    // jv_output =  "raw tx content" << std::endl << config::transaction(tx_);
    std::ostringstream config_tx;
    config_tx << config::transaction(tx_);
    jv_output = config_tx.str();

    if(argument_.send_flag){        
        if(blockchain.validate_transaction(tx_))
                throw tx_validate_exception{std::string("validate transaction failure")};
        if(blockchain.broadcast_transaction(tx_)) 
                throw tx_broadcast_exception{std::string("broadcast transaction failure")};
    }
    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

