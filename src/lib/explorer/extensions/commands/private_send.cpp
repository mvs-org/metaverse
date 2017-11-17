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
#include <boost/property_tree/ptree.hpp>      
#include <boost/property_tree/json_parser.hpp>

#include <metaverse/explorer/extensions/commands/private_send.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>
#include <metaverse/explorer/prop_tree.hpp>
#include <metaverse/explorer/extensions/exception.hpp> 

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."

/************************ deposit *************************/
console_result deposit::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if(!argument_.address.empty() && !blockchain.is_valid_address(argument_.address)) 
        throw address_invalid_exception{"invalid address!"};

    if (argument_.deposit != 7 && argument_.deposit != 30 
        && argument_.deposit != 90 && argument_.deposit != 182
        && argument_.deposit != 365)
    {
        throw account_deposit_period_exception{"deposit must be one in [7, 30, 90, 182, 365]."};
    }
    
    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr || pvaddr->empty()) 
        throw address_list_nullptr_exception{"nullptr for address list"};

    auto random = bc::pseudo_random();
    auto index = random % pvaddr->size();

    auto addr = argument_.address;
    if(addr.empty())
        addr = pvaddr->at(index).get_address();
        
    // receiver
    std::vector<receiver_record> receiver{
        {addr, "", argument_.amount, 0, utxo_attach_type::deposit, attachment()} 
    };
    auto deposit_helper = depositing_etp(*this, blockchain, std::move(auth_.name), std::move(auth_.auth), 
            std::move(addr), std::move(receiver), argument_.deposit, argument_.fee);
            
    deposit_helper.exec();

    // json output
    auto tx = deposit_helper.get_transaction();
    pt::write_json(output, config::prop_tree(tx, true));
    log::debug("command")<<"transaction="<<output.rdbuf();

    return console_result::okay;
}

/************************ send *************************/

console_result send::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if (!blockchain.is_valid_address(argument_.address))
        throw address_invalid_exception{std::string("invalid address : ") + argument_.address};
    
    // receiver
    std::vector<receiver_record> receiver{
        {argument_.address, "", argument_.amount, 0, utxo_attach_type::etp, attachment()}  
    };
    if(!argument_.memo.empty())
        receiver.push_back({argument_.address, "", 0, 0, utxo_attach_type::message, attachment(0, 0, blockchain_message(argument_.memo))});
    auto send_helper = sending_etp(*this, blockchain, std::move(auth_.name), std::move(auth_.auth), 
            "", std::move(receiver), argument_.fee);
    
    send_helper.exec();

    // json output
    auto tx = send_helper.get_transaction();
    pt::write_json(output, config::prop_tree(tx, true));
    log::debug("command")<<"transaction="<<output.rdbuf();

    return console_result::okay;
}

/************************ sendmore *************************/

console_result sendmore::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if (!argument_.mychange_address.empty() && !blockchain.is_valid_address(argument_.mychange_address))
        throw toaddress_invalid_exception{std::string("invalid address!") + argument_.mychange_address};
    //if (!blockchain.get_account_address(auth_.name, argument_.mychange_address))
        //throw argument_legality_exception{argument_.mychange_address + std::string(" not owned to ") + auth_.name};
    // receiver
    receiver_record record;
    std::vector<receiver_record> receiver;
    
    for( auto& each : argument_.receivers){
        colon_delimited2_item<std::string, uint64_t> item(each);
        record.target = item.first();
        // address check
        if (!blockchain.is_valid_address(record.target))
            throw toaddress_invalid_exception{std::string("invalid address!") + record.target};
        record.symbol = "";
        record.amount = item.second();
        if (!record.amount)
            throw argument_legality_exception{std::string("invalid amount parameter!") + each};
        record.asset_amount = 0;
        record.type = utxo_attach_type::etp; // attach not used so not do attah init
        receiver.push_back(record);
    }
    auto send_helper = sending_etp_more(*this, blockchain, std::move(auth_.name), std::move(auth_.auth), 
            "", std::move(receiver), std::move(argument_.mychange_address), argument_.fee);
    
    send_helper.exec();

    // json output
    auto tx = send_helper.get_transaction();
    pt::write_json(output, config::prop_tree(tx, true));
    log::debug("command")<<"transaction="<<output.rdbuf();

    return console_result::okay;
}

/************************ sendfrom *************************/
console_result sendfrom::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if(!blockchain.is_valid_address(argument_.from)) 
        throw fromaddress_invalid_exception{"invalid from address!"};
    if(!blockchain.is_valid_address(argument_.to)) 
        throw toaddress_invalid_exception{"invalid to address!"};
    
    // receiver
    std::vector<receiver_record> receiver{
        {argument_.to, "", argument_.amount, 0, utxo_attach_type::etp, attachment()}  
    };
    if(!argument_.memo.empty())
        receiver.push_back({argument_.to, "", 0, 0, utxo_attach_type::message, attachment(0, 0, blockchain_message(argument_.memo))});
    
    auto send_helper = sending_etp(*this, blockchain, std::move(auth_.name), std::move(auth_.auth), 
            std::move(argument_.from), std::move(receiver), argument_.fee);
    
    send_helper.exec();

    // json output
    auto tx = send_helper.get_transaction();
    pt::write_json(output, config::prop_tree(tx, true));
    log::debug("command")<<"transaction="<<output.rdbuf();

    return console_result::okay;
}

/************************ sendwithmsg *************************/

console_result sendwithmsg::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    // receiver
    std::vector<receiver_record> receiver{
        {argument_.address, "", argument_.amount, 0, utxo_attach_type::etp, attachment()},  
        {argument_.address, "", 0, 0, utxo_attach_type::message, attachment(0, 0, blockchain_message(argument_.message))}  
    };
    auto send_helper = sending_etp(*this, blockchain, std::move(auth_.name), std::move(auth_.auth), 
            "", std::move(receiver), argument_.fee);
    
    send_helper.exec();

    // json output
    auto tx = send_helper.get_transaction();
    pt::write_json(output, config::prop_tree(tx, true));
    log::debug("command")<<"transaction="<<output.rdbuf();

    return console_result::okay;
}

/************************ sendwithmsgfrom *************************/

console_result sendwithmsgfrom::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if(!blockchain.is_valid_address(argument_.from)) 
        throw fromaddress_invalid_exception{"invalid from address!"};
    if(!blockchain.is_valid_address(argument_.to)) 
        throw toaddress_invalid_exception{"invalid to address!"};
    
    // receiver
    std::vector<receiver_record> receiver{
        {argument_.to, "", argument_.amount, 0, utxo_attach_type::etp, attachment()},  
        {argument_.to, "", 0, 0, utxo_attach_type::message, attachment(0, 0, blockchain_message(argument_.message))}  
    };
    auto send_helper = sending_etp(*this, blockchain, std::move(auth_.name), std::move(auth_.auth), 
            std::move(argument_.from), std::move(receiver), argument_.fee);
    
    send_helper.exec();

    // json output
    auto tx = send_helper.get_transaction();
    pt::write_json(output, config::prop_tree(tx, true));
    log::debug("command")<<"transaction="<<output.rdbuf();

    return console_result::okay;
}

/************************ createmultisigtx *************************/

console_result createmultisigtx::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if(!blockchain.is_valid_address(argument_.from)) 
        throw fromaddress_invalid_exception{"invalid from address!"};
    
    auto addr = bc::wallet::payment_address(argument_.from);
    if(addr.version() != 0x05) // for multisig address
        throw fromaddress_invalid_exception{"from address is not script address."};
    if(!blockchain.is_valid_address(argument_.to)) 
        throw toaddress_invalid_exception{"invalid to address!"};
    
    account_multisig acc_multisig;
    if(!(acc->get_multisig_by_address(acc_multisig, argument_.from)))
        throw multisig_notfound_exception{"from address multisig record not found."};
    // receiver
    std::vector<receiver_record> receiver{
        {argument_.to, "", argument_.amount, 0, utxo_attach_type::etp, attachment()}  
    };
    auto send_helper = sending_multisig_etp(*this, blockchain, std::move(auth_.name), std::move(auth_.auth), 
            std::move(argument_.from), std::move(receiver), argument_.fee, 
            acc_multisig);
    
    send_helper.exec();

    // json output
    auto tx = send_helper.get_transaction();

    //pt::write_json(output, config::prop_tree(tx, true));
    //output << "raw tx content" << std::endl << config::transaction(tx);
    output << config::transaction(tx);
    return console_result::okay;
}

/************************ signmultisigtx *************************/

console_result signmultisigtx::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    // get not signed tx
    //output << "###### raw tx ######" << std::endl;
    //pt::write_json(output, config::prop_tree(argument_.transaction, true));
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
        
    //pt::write_json(output, config::prop_tree(tx_, true));
    //output << "raw tx content" << std::endl << config::transaction(tx_);
    output << config::transaction(tx_);

    if(argument_.send_flag){        
        if(blockchain.validate_transaction(tx_))
                throw tx_validate_exception{std::string("validate transaction failure")};
        if(blockchain.broadcast_transaction(tx_)) 
                throw tx_broadcast_exception{std::string("broadcast transaction failure")};
    }
    return console_result::okay;
}

/************************ issue *************************/

console_result issue::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();

    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(argument_.symbol);

    if(argument_.fee < 1000000000)
        throw asset_issue_poundage_exception{"issue asset fee less than 1000000000!"};
    if (argument_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw asset_symbol_length_exception{"asset symbol length must be less than 64."};
    // fail if asset is already in blockchain
    if(blockchain.is_asset_exist(argument_.symbol, false))
        throw asset_symbol_existed_exception{"asset symbol is already exist in blockchain"};
    // local database asset check
    auto sh_asset = blockchain.get_account_unissued_asset(auth_.name, argument_.symbol);
    if(!sh_asset)
        throw asset_symbol_notfound_exception{argument_.symbol + " not found"};
    #if 0
    if(asset_detail::asset_detail_type::created != sh_asset->at(0).detail.get_asset_type())
        throw asset_symbol_duplicate_exception{argument_.symbol + " has been issued"};
    #endif

    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr || pvaddr->empty()) 
        throw address_list_nullptr_exception{"nullptr for address list"};
    
    // get random address    
    auto index = bc::pseudo_random() % pvaddr->size();
    auto addr = pvaddr->at(index).get_address();
    
    // receiver
    std::vector<receiver_record> receiver{
        {addr, argument_.symbol, 0, 0, utxo_attach_type::asset_issue, attachment()}  
    };
    auto issue_helper = issuing_asset(*this, blockchain, std::move(auth_.name), std::move(auth_.auth), 
            "", std::move(argument_.symbol), std::move(receiver), argument_.fee);
    
    issue_helper.exec();

    // json output
    auto tx = issue_helper.get_transaction();
    pt::write_json(output, config::prop_tree(tx, true));

    // change asset status
    #if 0
    sh_asset->at(0).detail.set_asset_type(asset_detail::asset_detail_type::issued_not_in_blockchain);
    auto detail = std::make_shared<asset_detail>(sh_asset->at(0).detail);
    blockchain.store_account_asset(detail);
    #endif
    log::debug("command")<<"transaction="<<output.rdbuf();

    return console_result::okay;
}

/************************ issuefrom *************************/

console_result issuefrom::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(argument_.symbol);

    if(argument_.fee < 1000000000)
        throw asset_issue_poundage_exception{"issue asset fee less than 1000000000!"};
    if (argument_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw asset_symbol_length_exception{"asset symbol length must be less than 64."};
    if (!blockchain.is_valid_address(argument_.address))
        throw address_invalid_exception{"invalid address parameter!"};
    // fail if asset is already in blockchain
    if(blockchain.is_asset_exist(argument_.symbol, false))
        throw asset_symbol_existed_exception{"asset symbol is already exist in blockchain"};

    // local database asset check
    auto sh_asset = blockchain.get_account_unissued_asset(auth_.name, argument_.symbol);
    if(!sh_asset)
        throw asset_symbol_notfound_exception{argument_.symbol + " not found"};
    #if 0
    if(asset_detail::asset_detail_type::created != sh_asset->at(0).detail.get_asset_type())
        throw asset_symbol_duplicate_exception{argument_.symbol + " has been issued"};
    #endif

    // receiver
    std::vector<receiver_record> receiver{
        {argument_.address, argument_.symbol, 0, 0, utxo_attach_type::asset_issue, attachment()}  
    };
    auto issue_helper = issuing_asset(*this, blockchain, std::move(auth_.name), std::move(auth_.auth), 
            std::move(argument_.address), std::move(argument_.symbol), std::move(receiver), argument_.fee);
    
    issue_helper.exec();
    // json output
    auto tx = issue_helper.get_transaction();
#if 0
    auto issue_helper = issuing_locked_asset(*this, blockchain, std::move(auth_.name), std::move(auth_.auth), 
            std::move(argument_.address), std::move(argument_.symbol), std::move(receiver), argument_.fee, argument_.lockedtime);
    
    issue_helper.exec();
    // json output
    auto tx = issue_helper.get_transaction();
#endif
    // change asset status
    #if 0
    sh_asset->at(0).detail.set_asset_type(asset_detail::asset_detail_type::issued_not_in_blockchain);
    auto detail = std::make_shared<asset_detail>(sh_asset->at(0).detail);
    blockchain.store_account_asset(detail);
    #endif

    pt::write_json(output, config::prop_tree(tx, true));
    log::debug("command")<<"transaction="<<output.rdbuf();

    return console_result::okay;
}

/************************ issuemore *************************/

console_result issuemore::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ issuemorefrom *************************/

console_result issuemorefrom::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}

/************************ sendasset *************************/

console_result sendasset::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(argument_.symbol);
    
    if (argument_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw asset_symbol_length_exception{"asset symbol length must be less than 64."};
    if (!blockchain.is_valid_address(argument_.address))
        throw address_invalid_exception{"invalid to address parameter!"};
    if (!argument_.amount)
        throw asset_amount_exception{"invalid asset amount parameter!"};

    // receiver
    std::vector<receiver_record> receiver{
        {argument_.address, argument_.symbol, 0, argument_.amount, utxo_attach_type::asset_transfer, attachment()}  
    };
    auto send_helper = sending_asset(*this, blockchain, std::move(auth_.name), std::move(auth_.auth), 
            "", std::move(argument_.symbol), std::move(receiver), argument_.fee);
#if 0
    auto send_helper = sending_locked_asset(*this, blockchain, std::move(auth_.name), std::move(auth_.auth), 
            "", std::move(argument_.symbol), std::move(receiver), argument_.fee, argument_.lockedtime);
#endif
    
    send_helper.exec();

    // json output
    auto tx = send_helper.get_transaction();
    pt::write_json(output, config::prop_tree(tx, true));
    log::debug("command")<<"transaction="<<output.rdbuf();

    return console_result::okay;
}

/************************ sendassetfrom *************************/

console_result sendassetfrom::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(argument_.symbol);
    
    if (argument_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw asset_symbol_length_exception{"asset symbol length must be less than 64."};
    
    if (!blockchain.is_valid_address(argument_.from))
        throw fromaddress_invalid_exception{"invalid from address parameter!"};
    if (!blockchain.is_valid_address(argument_.to))
        throw toaddress_invalid_exception{"invalid to address parameter!"};
    if (!argument_.amount)
        throw asset_amount_exception{"invalid asset amount parameter!"};

    // receiver
    std::vector<receiver_record> receiver{
        {argument_.to, argument_.symbol, 0, argument_.amount, utxo_attach_type::asset_transfer, attachment()}  
    };
    auto send_helper = sending_asset(*this, blockchain, std::move(auth_.name), std::move(auth_.auth), 
            std::move(argument_.from), std::move(argument_.symbol), std::move(receiver), argument_.fee);
    
    send_helper.exec();

    // json output
    auto tx = send_helper.get_transaction();
    pt::write_json(output, config::prop_tree(tx, true));
    log::debug("command")<<"transaction="<<output.rdbuf();

    return console_result::okay;
}

/************************ createrawtx *************************/
enum transaction_type : uint16_t
{
     transfer_etp = 1,
     deposit_etp,
     issue_asset,
     transfer_asset,
};

console_result createrawtx::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    
    uint16_t type;
    std::vector<std::string> senders;
    std::vector<std::string> receivers;
    std::string symbol;
    std::string mychange_address;
    uint64_t fee;

    if (!option_.mychange_address.empty() && !blockchain.is_valid_address(option_.mychange_address))
        throw toaddress_invalid_exception{std::string("invalid address ") + option_.mychange_address};

    for (auto& each : option_.senders){
        // filter script address
        if(blockchain.is_script_address(each))
            throw fromaddress_invalid_exception{std::string("invalid address ") + each};
    }
    auto send_helper = sending_etp_more(*this, blockchain, std::move(auth_.name), std::move(auth_.auth), 
            "", std::move(receiver), std::move(argument_.mychange_address), argument_.fee);
    
    send_helper.exec();

    // json output
    auto tx = send_helper.get_transaction();
    pt::write_json(output, config::prop_tree(tx, true));
    log::debug("command")<<"transaction="<<output.rdbuf();

    return console_result::okay;
}

/************************ signrawtx *************************/
// copy from src/lib/consensus/clone/script/script.h
static std::vector<unsigned char> satoshi_to_chunk(const int64_t& value)
{
    if(value == 0)
        return std::vector<unsigned char>();

    std::vector<unsigned char> result;
    const bool neg = value < 0;
    uint64_t absvalue = neg ? -value : value;

    while(absvalue)
    {
        result.push_back(absvalue & 0xff);
        absvalue >>= 8;
    }

    if (result.back() & 0x80)
        result.push_back(neg ? 0x80 : 0);
    else if (neg)
        result.back() |= 0x80;

    return result;
}

console_result signrawtx::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    tx_type tx_ = argument_.transaction;
    // sign tx
    {
        uint32_t index = 0;
        chain::transaction tx_temp;
        uint64_t tx_height;  
        
        //for (auto& fromeach : from_list_){
        for (auto& fromeach : tx_.inputs){
            
            if(!(blockchain.get_transaction(fromeach.previous_output.hash, tx_temp, tx_height)))
                throw argument_legality_exception{std::string("invalid transaction hash ") + encode_hash(fromeach.previous_output.hash)};
            
            auto output = tx_temp.outputs.at(fromeach.previous_output.index);
            // get address private key
            auto address = payment_address::extract(output.script);
            if (!address || (address.version() == 0x5)) // script address : maybe multisig
                throw argument_legality_exception{std::string("invalid script ") + config::script(output.script).to_string()};

            auto acc_addr = blockchain.get_account_address(auth_.name, address.encoded());

            if(!acc_addr)
                throw argument_legality_exception{std::string("not own address ") + address.encoded()};

            // paramaters
            explorer::config::hashtype sign_type;
            uint8_t hash_type = (signature_hash_algorithm)sign_type;

            bc::explorer::config::ec_private config_private_key(acc_addr->get_prv_key(auth_.auth)); // address private key
            const ec_secret& private_key =    config_private_key;    
            bc::wallet::ec_private ec_private_key(private_key, 0u, true);

            bc::explorer::config::script config_contract(output.script); // previous output script
            const bc::chain::script& contract = config_contract;

            // gen sign
            bc::endorsement endorse;
            if (!bc::chain::script::create_endorsement(endorse, private_key,
                contract, tx_, index, hash_type))
            {
                throw tx_sign_exception{"signrawtx sign failure"};
            }

            // do script
            auto&& public_key = ec_private_key.to_public();
            data_chunk public_key_data;
            public_key.to_data(public_key_data);
            bc::chain::script ss;
            ss.operations.push_back({bc::chain::opcode::special, endorse});
            ss.operations.push_back({bc::chain::opcode::special, public_key_data});
            
            // if pre-output script is deposit tx.
            if (contract.pattern() == bc::chain::script_pattern::pay_key_hash_with_lock_height) {
                uint64_t lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(
                    contract.operations);
                ss.operations.push_back({bc::chain::opcode::special, satoshi_to_chunk(lock_height)});
            }
            // set input script of this tx
            tx_.inputs[index].script = ss;
            //fromeach.script = ss;
            index++;
        }

    }

    // get raw tx
    std::ostringstream buffer;
    pt::write_json(buffer, config::prop_tree(tx_, true));
    log::trace("signrawtx=") << buffer.str();

    if(blockchain.validate_transaction(tx_))
            throw tx_validate_exception{std::string("validate transaction failure")};

    pt::ptree aroot;
    aroot.put("hash", encode_hash(tx_.hash()));
    std::ostringstream tx_buf;
    tx_buf << config::transaction(tx_);
    aroot.put("hex", tx_buf.str());
    
    pt::write_json(output, aroot);
    
    return console_result::okay;
}

/************************ broadcasttx *************************/

console_result broadcasttx::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    // get raw tx
    std::ostringstream buffer;
    pt::write_json(buffer, config::prop_tree(argument_.transaction, true));
    log::trace("broadcasttx=") << buffer.str();
    tx_type tx_ = argument_.transaction;
    
    if(blockchain.validate_transaction(tx_))
            throw tx_validate_exception{std::string("validate transaction failure")};
    if(blockchain.broadcast_transaction(tx_)) 
            throw tx_broadcast_exception{std::string("broadcast transaction failure")};

    pt::ptree aroot;
    aroot.put("result", "success");
    aroot.put("hash", encode_hash(tx_.hash()));
    pt::write_json(output, aroot);
    
    return console_result::okay;
}

} //commands
} // explorer
} // libbitcoin
