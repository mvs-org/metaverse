/**
 * Copyright (c) 2016 mvs developers 
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

#ifdef _WIN32
#include <boost/bind/placeholders.hpp>
using namespace boost::placeholders;
#endif

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <metaverse/explorer/command_assistant.hpp>
#include <boost/algorithm/string.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

// ---------------------------------------------------------------------------
std::string ec_to_xxx_impl(const char* commands, const std::string& fromkey, bool use_testnet_rules)
{
    std::ostringstream sout("");
    std::istringstream sin(fromkey);

    const char* cmds[]{commands, "-v", "127"};
    if (use_testnet_rules){
        if(dispatch_command(3, cmds, sin, sout, sout)){
            throw std::logic_error(sout.str());
        }
    } else {
        if(dispatch_command(1, cmds, sin, sout, sout)){
            throw std::logic_error(sout.str());
        }
    }

    return sout.str();
}

uint64_t get_total_payment_amount(const std::vector<std::string>& receiver_list,
        address_amount& mychange)
{
    uint64_t total_payment_amount = 0;

    std::vector<std::string> results;
    for (auto& iter : receiver_list)
    {
        results.clear();
        boost::split(results, iter, boost::is_any_of(":"));
        total_payment_amount += std::stoul(results[1], nullptr, 10);
    }

    // last one for mychange and fee
    auto fee = std::stoul(results[1], nullptr, 10);
    if (fee > utxo_helper::maximum_fee || fee < utxo_helper::minimum_fee)
        throw std::logic_error{"fee must in [10000, 10000000000]"};

    mychange = std::make_pair(results[0], fee);

    return total_payment_amount;
}

void get_tx_decode(const std::string& tx_set, std::string& tx_decode)
{
    std::ostringstream sout("");
    std::istringstream sin(tx_set);

    const char* cmds[]{"tx-decode"};
    if (dispatch_command(1, cmds, sin, sout, sout))
        throw std::logic_error(sout.str());

    tx_decode = sout.str();
}

void validate_tx(const std::string& tx_set)
{
    std::ostringstream sout("");
    std::istringstream sin(tx_set);

    const char* cmds[]{"validate-tx"};
    if (dispatch_command(1, cmds, sin, sout, sout)){
        log::debug(LOG_COMMAND)<<"validate-tx sout:"<<sout.str();
        throw std::logic_error(sout.str());
    }
}

void send_tx(const std::string& tx_set, std::string& send_ret)
{
    std::ostringstream sout("");
    std::istringstream sin(tx_set);

    const char* cmds[]{"send-tx"};
    if (dispatch_command(1, cmds, sin, sout, sout)){
        log::debug(LOG_COMMAND)<<"send-tx sout:"<<sout.str();
        throw std::logic_error(sout.str());
    }

    send_ret = sout.str();
}


// ---------------------------------------------------------------------------
bool utxo_helper::fetch_utxo(std::string& change, bc::blockchain::block_chain_impl& blockchain)
{
    using namespace boost::property_tree;

    uint64_t remaining = total_payment_amount_;
    uint32_t from_list_index = 1;
    for (auto& fromeach : from_list_){

        std::string&& frompubkey = ec_to_xxx_impl("ec-to-public", fromeach.first);
        std::string&& fromaddress = ec_to_xxx_impl("ec-to-address", frompubkey, is_testnet_rules);

        std::string amount = std::to_string(fromeach.second);

        // last one
        if (from_list_index == from_list_.size()){
            amount = std::to_string(remaining);
        }
        remaining -= fromeach.second;

        // exec
        const char* cmds[]{"xfetchutxo", amount.c_str(), fromaddress.c_str(), "-t", "etp"};

        std::ostringstream sout("");
        std::istringstream sin;
        if (dispatch_command(5, cmds, sin, sout, sout, blockchain)){
            throw std::logic_error(sout.str());
        }
        sin.str(sout.str());

        // parse json
        ptree pt; 
        read_json(sin, pt);

        change = pt.get<std::string>("change");
        auto points = pt.get_child("points");

        // not found
        if (points.size() == 0 && change == "0"){
            return false;
        }

        // last one
        if (from_list_index++ == from_list_.size()){
            set_mychange_by_threshold(change);
        }

        // found, then push_back
        tx_items tx;
        for (auto& i: points){
            tx.txhash.clear();
            tx.output.index.clear();

            tx.txhash = i.second.get<std::string>("hash");
            tx.output.index  = i.second.get<std::string>("index");

            keys_inputs_[fromeach.first].push_back(tx);
        }
    }

    return true;
}


// ---------------------------------------------------------------------------
bool utxo_helper::fetch_tx() 
{
    using namespace boost::property_tree;

    const char* cmds[]{"fetch-tx"};

    std::ostringstream sout;
    std::istringstream sin;

    for (auto& fromeach : from_list_){

        for (auto& iter : keys_inputs_[fromeach.first]){
            sout.str("");
            sin.str(iter.txhash);

            if (dispatch_command(1, cmds, sin, sout, sout))
                throw std::logic_error(sout.str());
            sin.str(sout.str());

            ptree pt;
            read_json(sin, pt);

            auto transaction = pt.get_child("transaction");
            auto outputs = transaction.get_child("outputs");

            // fill tx_items outputs
            auto target_pos = std::stoi(iter.output.index);
            int pos = 0;
            for (auto& i: outputs){
                if (target_pos == pos++){
                    iter.output.script = i.second.get<std::string>("script");
                    iter.output.value = i.second.get<uint64_t>("value");
                    break;
                }
            }
        }

    }

    return true;
}

void utxo_helper::get_tx_encode(std::string& tx_encode)
{
    const char* cmds[1024]{0x00};
    uint32_t i = 0;
    cmds[i++] = "tx-encode";
    auto&& period_str = std::to_string(reward_in_);

    if (reward_in_){
        cmds[i++] = "-s";
        cmds[i++] = "6"; //TODO, period deposit
        cmds[i++] = "-p";
        cmds[i++] = period_str.c_str();
        cmds[i++] = "-d";
        cmds[i++] = receiver_list_.front().c_str(); // send the deposit which shall locked in scripts
    }


    // input args
    uint64_t adjust_amount = 0;
    for (auto& fromeach : from_list_){
        for (auto& iter: keys_inputs_[fromeach.first]){
            iter.output.as_tx_encode_input_args = iter.txhash + ":" + iter.output.index;
            adjust_amount += iter.output.value;
            if (i >= 677) // limit in ~333 inputs
            {
                auto&& response = "Too many inputs limit, suggest less than " + std::to_string(adjust_amount) + " satoshi.";
                throw std::runtime_error(response);
            }

            cmds[i++] = "-i";
            cmds[i++] = iter.output.as_tx_encode_input_args.c_str();
        }
    }

    // output args
    for (auto& iter: receiver_list_) {
        if (i >= 687) {
                throw std::runtime_error{"Too many inputs/outputs makes tx too large, canceled."};
        }
        cmds[i++] = "-o";
        cmds[i++] = iter.c_str();
    }

    std::ostringstream sout;
    std::istringstream sin;
    if (dispatch_command(i, cmds, sin, sout, sout)){
        throw std::logic_error(sout.str());
    }

    log::debug(LOG_COMMAND)<<"tx-encode sout:"<<sout.str();
    tx_encode = sout.str();

}

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

uint32_t utxo_helper::get_reward_lock_block_height()
{
    int index;
    switch(reward_in_) {
        case 7 :
            index = 0;
            break;
        case 30 :
            index = 1;
            break;
        case 90 :
            index = 2;
            break;
        case 182 :
            index = 3;
            break;
        case 365 :
            index = 4;
            break;
        default :
            index = 0;
            break;
    }
    return (uint32_t)bc::consensus::lock_heights[index];
}

void utxo_helper::get_input_sign(std::string& tx_encode)
{
    bc::explorer::config::transaction config_tx(tx_encode);
    tx_type& tx = config_tx.data();

    uint32_t index = 0;
    for (auto& fromeach : from_list_){
        for (auto& iter: keys_inputs_[fromeach.first]){
            // paramaters
            explorer::config::hashtype sign_type;
            uint8_t hash_type = (signature_hash_algorithm)sign_type;

            bc::explorer::config::ec_private config_private_key(fromeach.first);
            const ec_secret& private_key =    config_private_key;    
            bc::wallet::ec_private ec_private_key(private_key, 0u, true);

            bc::explorer::config::script config_contract(iter.output.script);
            const bc::chain::script& contract = config_contract;

            // gen sign
            bc::endorsement endorse;
            if (!bc::chain::script::create_endorsement(endorse, private_key,
                contract, tx, index, hash_type))
            {
                throw std::logic_error{"get_input_sign sign failure"};
            }

            // do script
            auto&& public_key = ec_private_key.to_public();
            data_chunk public_key_data;
            public_key.to_data(public_key_data);
            bc::chain::script ss;
            ss.operations.push_back({bc::chain::opcode::special, endorse});
            ss.operations.push_back({bc::chain::opcode::special, public_key_data});

            // if pre-output script is deposit tx.
            if (contract.pattern() == bc::chain::script_pattern::pay_key_hash_with_lock_height)
            {
            uint64_t lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(
                    contract.operations);
                ss.operations.push_back({bc::chain::opcode::special, satoshi_to_chunk(lock_height)});
            }

            // set input script of this tx
            tx.inputs[index].script = ss;
            index++;
        }
    }

    std::ostringstream output_tx;
    output_tx<<config_tx;
    tx_encode = output_tx.str();

}



void utxo_helper::get_input_set(const std::string& tx_encode, std::string& tx_set)
{
    std::ostringstream sout(tx_encode);
    std::istringstream sin;

    int i = 0;
    for (auto& fromeach : from_list_){
        std::string&& frompubkey = ec_to_xxx_impl("ec-to-public", fromeach.first);

        for (auto& iter: keys_inputs_[fromeach.first]){
            std::string input_script;
            if (iter.output.script_version == 6u)
            {
                auto&& cret = satoshi_to_chunk(get_reward_lock_block_height());
                input_script = "[ " + iter.output.as_input_sign + " ] " + "[ " + frompubkey + " ] "
                        + "[ " + bc::encode_base16(cret) + " ]";
            }
            else
                input_script = "[ " + iter.output.as_input_sign + " ] " + "[ " + frompubkey + " ]";

            sin.str(sout.str());
            sout.str("");

            std::string&& tx_encode_index = std::to_string(i++);
            const char* cmds[]{"input-set", "-i", tx_encode_index.c_str(), input_script.c_str()};

            if (dispatch_command(4, cmds, sin, sout, sout))
                throw std::logic_error(sout.str());
        }
    }

    tx_set = sout.str();
    log::debug(LOG_COMMAND)<<"input-set sout:"<<tx_set;
}

uint64_t utxo_helper::get_my_balance()
{
    uint64_t total_balance = 0;
    for (auto& each : from_list_){
        total_balance += each.second;
    }
    return total_balance;
}

void utxo_helper::set_mychange_by_threshold(std::string& mychange)
{
    receiver_list_.pop_back();
    receiver_list_.push_back({mychange_.first + ":" + mychange});
}

void utxo_helper::group_utxo()
{
    auto balance = get_my_balance();
    if (balance < total_payment_amount_)
        throw std::logic_error{"no enough balance"};

    // some utxo can pay
    auto pos = std::find_if(from_list_.begin(), from_list_.end(), [this](const prikey_amount& i){
            return i.second >=  total_payment_amount_;
            });

    if (pos != from_list_.end()){
        auto pa = *pos;
        from_list_.clear();
        from_list_.push_back(pa);

        return;
    }

    // not found, group small changes
    uint64_t k = 0;
    for (auto iter = from_list_.begin(); iter != from_list_.end();){
        if (k > total_payment_amount_){
            iter = from_list_.erase(iter);
            continue;
        }

        k += iter->second;
        ++iter;
    }
}

// ---------------------------------------------------------------------------
bool send_impl(utxo_helper& utxo, bc::blockchain::block_chain_impl& blockchain, std::ostream& output, std::ostream& cerr)
{
    // initialization, may throw
    utxo.get_payment_by_receivers();

    // group send amount by address
    utxo.group_utxo();

    // get utxo in each address
    std::string change{""};
    if (!utxo.fetch_utxo(change, blockchain))
        return false;

    // load pre-transaction
    utxo.fetch_tx();

    // tx-encode
    std::string tx_encode;
    utxo.get_tx_encode(tx_encode);

    // input-sign and input-set
    utxo.get_input_sign(tx_encode);

    //// input-set
    //std::string tx_set;
    //utxo.get_input_set(tx_encode, tx_set);

    // validate-tx
    validate_tx(tx_encode);

    // send-tx
    std::string send_ret;
    send_tx(tx_encode, send_ret);

    //// tx-decode
    std::string tx_decode;
    get_tx_decode(tx_encode, tx_decode);
    output<<tx_decode;

    return true;
}

// ---------------------------------------------------------------------------
bool utxo_attach_issue_helper::fetch_utxo(std::string& change, bc::blockchain::block_chain_impl& blockchain)
{
    using namespace boost::property_tree;
    
    std::string&& amount = std::to_string(total_payment_amount_);
    bool found = false;
    // 1. find one address whose all utxo balance is bigger than total_payment_amount_
    for (auto& fromeach : from_list_){

        std::string&& frompubkey = ec_to_xxx_impl("ec-to-public", fromeach.first);
        std::string&& fromaddress = ec_to_xxx_impl("ec-to-address", frompubkey);

        //std::string&& amount = std::to_string(fromeach.second);

        const char* cmds[]{"xfetchutxo", amount.c_str(), fromaddress.c_str()};

        std::ostringstream sout("");
        std::istringstream sin;
        if (dispatch_command(3, cmds, sin, sout, sout, blockchain)){
            throw std::logic_error(sout.str());
        }
        sin.str(sout.str());

        // parse json
        ptree pt; 
        read_json(sin, pt);

        change = pt.get<std::string>("change");
        auto points = pt.get_child("points");

        // not found, try next address 
        if (points.size() == 0 && change == "0"){
            //return false;
            continue;
        }

        // found, then push_back
        tx_items tx;
        for (auto& i: points){
            tx.txhash.clear();
            tx.output.index.clear();

            tx.txhash = i.second.get<std::string>("hash");
            tx.output.index  = i.second.get<std::string>("index");

            keys_inputs_[fromeach.first].push_back(tx);
        }
        mychange_.first = fromaddress;
        mychange_.second = std::stoull(change);
        //set_mychange_by_threshold();
        found = true;
        break; // found one then break

    }

    // check the utxo whose amount>total_payment_amount_ exists  
    if(found)
        return true;
    
    // 2. get all utxo of address whose balance is samller than total_payment_amount_, but all address balance sum 
    //    is bigger than total_payment_amount_
    found = false;
    mychange_.first = "";
    mychange_.second = 0;
    for (auto& fromeach : from_list_){

        std::string&& frompubkey = ec_to_xxx_impl("ec-to-public", fromeach.first);
        std::string&& fromaddress = ec_to_xxx_impl("ec-to-address", frompubkey);

        const char* cmds[]{"xfetchutxo", "0", fromaddress.c_str()};

        std::ostringstream sout("");
        std::istringstream sin;
        if (dispatch_command(3, cmds, sin, sout, sout, blockchain)){
            throw std::logic_error(sout.str());
        }
        sin.str(sout.str());

        // parse json
        ptree pt; 
        read_json(sin, pt);

        change = pt.get<std::string>("change");
        auto points = pt.get_child("points");

        // not found, try next address 
        if (points.size() == 0 && change == "0"){
            //return false;
            continue;
        }

        // found, then push_back
        tx_items tx;
        for (auto& i: points){
            tx.txhash.clear();
            tx.output.index.clear();

            tx.txhash = i.second.get<std::string>("hash");
            tx.output.index  = i.second.get<std::string>("index");

            keys_inputs_[fromeach.first].push_back(tx);
        }
        mychange_.first = fromaddress;
        mychange_.second += std::stoull(change); // just used for sum all address utxo balance
    }
    if(mychange_.second >= total_payment_amount_) {
        mychange_.second -= total_payment_amount_;
        found = true;
    }
    // check all the address utxo balance sum>=total_payment_amount_ exists  
    if(found)
        return true;
    return false;
}


// ---------------------------------------------------------------------------
bool utxo_attach_issue_helper::fetch_tx() 
{
    using namespace boost::property_tree;

    const char* cmds[]{"fetch-tx"};

    std::ostringstream sout;
    std::istringstream sin;

    for (auto& fromeach : from_list_){

        for (auto& iter : keys_inputs_[fromeach.first]){
            sout.str("");
            sin.str(iter.txhash);

            if (dispatch_command(1, cmds, sin, sout, sout))
                throw std::logic_error(sout.str());
            sin.str(sout.str());

            ptree pt;
            read_json(sin, pt);

            auto transaction = pt.get_child("transaction");
            auto outputs = transaction.get_child("outputs");

            // fill tx_items outputs
            auto target_pos = std::stoi(iter.output.index);
            int pos = 0;
            for (auto& i: outputs){
                if (target_pos == pos++){
                    iter.output.script = i.second.get<std::string>("script");
                    iter.output.value = i.second.get<uint64_t>("value");
                    break;
                }
            }
        }

    }

    return true;
}
void utxo_attach_issue_helper::get_tx_encode(std::string& tx_encode, bc::blockchain::block_chain_impl& blockchain)
{
    const char* cmds[1024]{0x00};
    int i = 0;
    cmds[i++] = "encodeattachtx";
    cmds[i++] = name_.c_str();
    cmds[i++] = passwd_.c_str();

    // input args
    for (auto& fromeach : from_list_){
        for (auto& iter: keys_inputs_[fromeach.first]){ // only one address utxo inputs
            iter.output.as_tx_encode_input_args = iter.txhash + ":" + iter.output.index;
            cmds[i++] = "-i";
            cmds[i++] = iter.output.as_tx_encode_input_args.c_str();
        }
    }
    // add mychange to receiver list
    //receiver_list_.push_back({mychange_.first, 1, business_type_, "", 0, mychange_.second, ""});
    receiver_list_.push_back({mychange_.first, 1, business_type_, symbol_, amount_, mychange_.second, ""});
    // output args
    for (auto& iter: receiver_list_) {
        cmds[i++] = "-o";
        get_utxo_option(iter);
        cmds[i++] = iter.output_option.c_str();
    }

    std::ostringstream sout;
    std::istringstream sin;
    if (dispatch_command(i, cmds, sin, sout, sout, blockchain))
        throw std::logic_error(sout.str());

    log::debug(LOG_COMMAND)<<"encodeattachtx sout:"<<sout.str();
    tx_encode = sout.str();
}

void utxo_attach_issue_helper::get_input_sign(std::string& tx_encode)
{
    std::ostringstream sout;
    std::istringstream sin;

    int i = 0;
    for (auto& fromeach : from_list_){
        for (auto& iter: keys_inputs_[fromeach.first]){
            sin.str(tx_encode);
            sout.str("");

            std::string&& tx_encode_index = std::to_string(i++);
            const char* cmds[]{"input-sign", "-i", tx_encode_index.c_str(), fromeach.first.c_str(), iter.output.script.c_str()};
            if (dispatch_command(5, cmds, sin, sout, sout))
                throw std::logic_error(sout.str());
            iter.output.as_input_sign = sout.str();
            log::debug(LOG_COMMAND)<<"input-sign sout:"<<iter.output.as_input_sign;
        }
    }
}

void utxo_attach_issue_helper::get_input_set(const std::string& tx_encode, std::string& tx_set)
{
    std::ostringstream sout(tx_encode);
    std::istringstream sin;

    int i = 0;
    for (auto& fromeach : from_list_){
        std::string&& frompubkey = ec_to_xxx_impl("ec-to-public", fromeach.first);

        for (auto& iter: keys_inputs_[fromeach.first]){
            std::string&& input_script = "[ " + iter.output.as_input_sign + " ] " + "[ " + frompubkey + " ]";
            sin.str(sout.str());
            sout.str("");

            std::string&& tx_encode_index = std::to_string(i++);
            const char* cmds[]{"input-set", "-i", tx_encode_index.c_str(), input_script.c_str()};

            if (dispatch_command(4, cmds, sin, sout, sout))
                throw std::logic_error(sout.str());
        }
    }

    tx_set = sout.str();
    log::debug(LOG_COMMAND)<<"input-set sout:"<<tx_set;
}

uint64_t utxo_attach_issue_helper::get_my_balance()
{
    uint64_t total_balance = 0;
    for (auto& each : from_list_){
        total_balance += each.second;
    }
    return total_balance;
}
bool utxo_attach_issue_helper::group_utxo()
{
    bool ret = false;
    auto balance = get_my_balance();
    if (balance < total_payment_amount_)
        throw std::logic_error{"Account don't have enough balances"};

    // some utxo can pay
    auto pos = std::find_if(from_list_.begin(), from_list_.end(), [this](const prikey_amount& i){
            return i.second >=  total_payment_amount_;
            });

    if (pos != from_list_.end()){
        auto pa = *pos;
        from_list_.clear();
        from_list_.push_back(pa);
        //set_mychange_by_threshold(pa.first, pa.second);
        return true;
    }

    // not found, group small changes
    uint64_t k = 0;
    for (auto iter = from_list_.begin(); iter != from_list_.end(); ++iter){
        if (k > total_payment_amount_){
            iter = from_list_.erase(iter);
            continue;
        }

        k += iter->second;
    }
    if (k > total_payment_amount_) {
        //set_mychange_by_threshold(from_list_.begin()->first, k);
        ret = true;
    } 
    return ret;
}

void utxo_attach_issue_helper::get_utxo_option(utxo_attach_info& info)
{
    
    /*    
    target:1:etp                  :amount(value)  // 4
    target:1:etp-award              :amount(value)  // 4
    target:1:asset-issue   :symbol:value          // 4
    target:1:asset-transfer:symbol:amount:value   // 5
    */
    #define  PARM_SEPARATOR  ":"
    std::string ret = "";
    std::string type = info.type;
    
    if(type == "etp" || type == "etp-award" ) {
        ret = info.target + PARM_SEPARATOR 
            + std::to_string(info.version) + PARM_SEPARATOR
            + type + PARM_SEPARATOR
            + std::to_string(info.value);
    } else if(type == "asset-issue") {
        ret = info.target + PARM_SEPARATOR 
            + std::to_string(info.version) + PARM_SEPARATOR
            + type + PARM_SEPARATOR
            + info.symbol + PARM_SEPARATOR
            + std::to_string(info.value);
    } else if(type == "asset-transfer") {
        ret = info.target + PARM_SEPARATOR 
            + std::to_string(info.version) + PARM_SEPARATOR
            + type + PARM_SEPARATOR
            + info.symbol + PARM_SEPARATOR
            + std::to_string(info.amount) + PARM_SEPARATOR
            + std::to_string(info.value);
    } 

    info.output_option = ret;
}
bool send_impl(utxo_attach_issue_helper& utxo, bc::blockchain::block_chain_impl& blockchain, std::ostream& output, std::ostream& cerr)
{
    //utxo_attach_issue_helper utxo(std::move(name), std::move(passwd), std::move(type), std::move(from), std::move(receiver), fee);

    // initialization, may throw
    utxo.get_payment_by_receivers();

    // clean from_list_ by some algorithm
    if(!utxo.group_utxo())
        throw std::logic_error{"not enough etp in account addresses"};
        //return false; // if not enough etp to pay

    // get utxo
    std::string change{""};
    if (!utxo.fetch_utxo(change, blockchain))
        throw std::logic_error{"not enough etp in utxo of some address"};
        //return false;

    // load pre-transaction
    utxo.fetch_tx();

    // tx-encode
    std::string tx_encode;
    utxo.get_tx_encode(tx_encode, blockchain);

    // input-sign
    utxo.get_input_sign(tx_encode);

    // input-set
    std::string tx_set;
    utxo.get_input_set(tx_encode, tx_set);

    // tx-decode
    std::string tx_decode;
    get_tx_decode(tx_set, tx_decode);

    // validate-tx
    validate_tx(tx_set);

    // send-tx
    std::string send_ret;
    send_tx(tx_set, send_ret);

    //output<<"{\"sent-result\":\"" << send_ret <<"\",";
    output<< tx_decode ;

    return true;
}

// ----------------------------------------------------------------------------

bool utxo_attach_send_helper::fetch_utxo_impl(bc::blockchain::block_chain_impl& blockchain,
    std::string& prv_key, uint64_t payment_amount, uint64_t& utxo_change)
{
    using namespace boost::property_tree;

    std::string&& amount = std::to_string(payment_amount);
    std::string&& frompubkey = ec_to_xxx_impl("ec-to-public", prv_key);
    std::string&& fromaddress = ec_to_xxx_impl("ec-to-address", frompubkey);

    const char* cmds[]{"xfetchutxo", amount.c_str(), fromaddress.c_str()};

    std::ostringstream sout("");
    std::istringstream sin;
    if (dispatch_command(3, cmds, sin, sout, sout, blockchain)){
        throw std::logic_error(sout.str());
    }
    sin.str(sout.str());

    // parse json
    ptree pt; 
    read_json(sin, pt);

    std::string change = pt.get<std::string>("change");
    auto points = pt.get_child("points");

    // not found, return  
    if (points.size() == 0 && change == "0"){
            return false;
    }

    // found, then push_back
    tx_items tx;
    for (auto& i: points){
        tx.txhash.clear();
        tx.output.index.clear();

        tx.txhash = i.second.get<std::string>("hash");
        tx.output.index  = i.second.get<std::string>("index");

        bool exist = false;
        for(auto& item: keys_inputs_[prv_key]) {
            if((item.txhash == tx.txhash) && (item.output.index == tx.output.index))
                exist = true;
        }

        
        if(!exist)
            keys_inputs_[prv_key].push_back(tx);
    }
    utxo_change = std::stoull(change);
    
    return true;
    
}

bool utxo_attach_send_helper::fetch_utxo(bc::blockchain::block_chain_impl& blockchain)
{    
    uint64_t change = 0, utxo_change = 0;
    for (auto& fromeach : etp_ls_){
        fetch_utxo_impl(blockchain, fromeach.first, 0, change);
        utxo_change += change;
    }
    
    for (auto& fromeach : asset_ls_){ 
        fetch_utxo_impl(blockchain, fromeach.key, 0, change);
        utxo_change += change;
    }
    
    // fill reback change and asset amount
    std::string&& frompubkey = ec_to_xxx_impl("ec-to-public", asset_ls_.begin()->key);
    std::string&& fromaddress = ec_to_xxx_impl("ec-to-address", frompubkey);
    set_mychange(fromaddress, utxo_change - total_payment_amount_);
    
    return true; 
}

// ---------------------------------------------------------------------------
bool utxo_attach_send_helper::fetch_tx() 
{
    using namespace boost::property_tree;

    const char* cmds[]{"fetch-tx"};

    std::ostringstream sout;
    std::istringstream sin;

    for (auto& fromeach : etp_ls_){

        for (auto& iter : keys_inputs_[fromeach.first]){
            sout.str("");
            sin.str(iter.txhash);

            if (dispatch_command(1, cmds, sin, sout, sout))
                throw std::logic_error(sout.str());
            sin.str(sout.str());

            ptree pt;
            read_json(sin, pt);

            auto transaction = pt.get_child("transaction");
            auto outputs = transaction.get_child("outputs");

            // fill tx_items outputs
            auto target_pos = std::stoi(iter.output.index);
            int pos = 0;
            for (auto& i: outputs){
                if (target_pos == pos++){
                    iter.output.script = i.second.get<std::string>("script");
                    iter.output.value = i.second.get<uint64_t>("value");
                    break;
                }
            }
        }

    }
    
    for (auto& fromeach : asset_ls_){

        for (auto& iter : keys_inputs_[fromeach.key]){
            sout.str("");
            sin.str(iter.txhash);

            if (dispatch_command(1, cmds, sin, sout, sout))
                throw std::logic_error(sout.str());
            sin.str(sout.str());

            ptree pt;
            read_json(sin, pt);

            auto transaction = pt.get_child("transaction");
            auto outputs = transaction.get_child("outputs");

            // fill tx_items outputs
            auto target_pos = std::stoi(iter.output.index);
            int pos = 0;
            for (auto& i: outputs){
                if (target_pos == pos++){
                    iter.output.script = i.second.get<std::string>("script");
                    iter.output.value = i.second.get<uint64_t>("value");
                    break;
                }
            }
        }

    }

    return true;
}
void utxo_attach_send_helper::generate_receiver_list()
{
    auto total_amount = get_asset_amounts();
    receiver_list_.push_back({mychange_.first, 1, "asset-transfer", symbol_, total_amount-amount_, mychange_.second, ""});
    receiver_list_.push_back({address_, 1, "asset-transfer", symbol_, amount_, 0, ""});
}
bool utxo_attach_send_helper::is_cmd_exist(const char* cmds[], size_t len, char* cmd)
{
    size_t i = 0;
    
    for( i=0; i<len; i++) {
        if(cmds[i] && cmd && (0 == std::string(cmds[i]).compare(std::string(cmd))))
            return true;
    }
    return false;
}
void utxo_attach_send_helper::get_tx_encode(std::string& tx_encode, bc::blockchain::block_chain_impl& blockchain)
{
    const char* cmds[1024]{0x00};
    int i = 0;
    cmds[i++] = "encodeattachtx";
    cmds[i++] = name_.c_str();
    cmds[i++] = passwd_.c_str();

    // input args
    for (auto& fromeach : etp_ls_){
        for (auto& iter: keys_inputs_[fromeach.first]){ // only one address utxo inputs
            iter.output.as_tx_encode_input_args = iter.txhash + ":" + iter.output.index;
            if(is_cmd_exist(cmds, sizeof(cmds)/sizeof(cmds[0]), const_cast<char*>(iter.output.as_tx_encode_input_args.c_str())))
                continue;
            cmds[i++] = "-i";
            cmds[i++] = iter.output.as_tx_encode_input_args.c_str();
        }
    }

    for (auto& fromeach : asset_ls_){
        for (auto& iter: keys_inputs_[fromeach.key]){ // only one address utxo inputs
            iter.output.as_tx_encode_input_args = iter.txhash + ":" + iter.output.index;
            if(is_cmd_exist(cmds, sizeof(cmds)/sizeof(cmds[0]), const_cast<char*>(iter.output.as_tx_encode_input_args.c_str())))
                continue;
            cmds[i++] = "-i";
            cmds[i++] = iter.output.as_tx_encode_input_args.c_str();
        }
    }
    
    // generate receiver list according from list
    generate_receiver_list();
    
    // output args
    for (auto& iter: receiver_list_) {
        cmds[i++] = "-o";
        get_utxo_option(iter);
        cmds[i++] = iter.output_option.c_str();
    }

    std::ostringstream sout;
    std::istringstream sin;
    if (dispatch_command(i, cmds, sin, sout, sout, blockchain))
        throw std::logic_error(sout.str());

    log::debug(LOG_COMMAND)<<"encodeattachtx sout:"<<sout.str();
    tx_encode = sout.str();
}

void utxo_attach_send_helper::get_input_sign(std::string& tx_encode)
{
    std::ostringstream sout;
    std::istringstream sin;
    
    const char* bank_cmds[1024]{0x00};

    int i = 0;
    for (auto& fromeach : etp_ls_){
        for (auto& iter: keys_inputs_[fromeach.first]){
            if(!iter.output.as_input_sign.empty())  // only assign once
                continue;
            sin.str(tx_encode);
            sout.str("");

            std::string&& tx_encode_index = std::to_string(i++);
            
            if(is_cmd_exist(bank_cmds, sizeof(bank_cmds)/sizeof(bank_cmds[0]), const_cast<char*>((tx_encode_index + fromeach.first + iter.output.script).c_str())))
                continue;
            bank_cmds[i] = ((tx_encode_index + fromeach.first + iter.output.script).c_str());
            const char* cmds[]{"input-sign", "-i", tx_encode_index.c_str(), fromeach.first.c_str(), iter.output.script.c_str()};
            log::debug(LOG_COMMAND)<<"etp input-sign="<<tx_encode_index<<" "<<fromeach.first<<" "<<iter.output.script;
            if (dispatch_command(5, cmds, sin, sout, sout))
                throw std::logic_error(sout.str());
            iter.output.as_input_sign = sout.str();
            log::debug(LOG_COMMAND)<<"input-sign sout:"<<iter.output.as_input_sign;
        }
    }
    
    for (auto& fromeach : asset_ls_){
        for (auto& iter: keys_inputs_[fromeach.key]){
            if(!iter.output.as_input_sign.empty())  // only assign once
                continue;
            sin.str(tx_encode);
            sout.str("");

            std::string&& tx_encode_index = std::to_string(i++);
            
            if(is_cmd_exist(bank_cmds, sizeof(bank_cmds)/sizeof(bank_cmds[0]), const_cast<char*>((tx_encode_index + fromeach.key + iter.output.script).c_str())))
                continue;
            bank_cmds[i] = ((tx_encode_index + fromeach.key + iter.output.script).c_str());
            const char* cmds[]{"input-sign", "-i", tx_encode_index.c_str(), fromeach.key.c_str(), iter.output.script.c_str()};
            log::debug(LOG_COMMAND)<<"asset input-sign="<<tx_encode_index<<" "<<fromeach.key<<" "<<iter.output.script;
            if (dispatch_command(5, cmds, sin, sout, sout))
                throw std::logic_error(sout.str());
            iter.output.as_input_sign = sout.str();
            log::debug(LOG_COMMAND)<<"input-sign sout:"<<iter.output.as_input_sign;
        }
    }
}

void utxo_attach_send_helper::get_input_set(const std::string& tx_encode, std::string& tx_set)
{
    std::ostringstream sout(tx_encode);
    std::istringstream sin;
    const char* bank_cmds[1024]{0x00};

    int i = 0;
    for (auto& fromeach : etp_ls_){
        std::string&& frompubkey = ec_to_xxx_impl("ec-to-public", fromeach.first);

        for (auto& iter: keys_inputs_[fromeach.first]){
            if(iter.output.as_input_sign.empty())
                continue;
            std::string&& input_script = "[ " + iter.output.as_input_sign + " ] " + "[ " + frompubkey + " ]";
            if(is_cmd_exist(bank_cmds, sizeof(bank_cmds)/sizeof(bank_cmds[0]), const_cast<char*>((input_script).c_str())))
                continue;
            bank_cmds[i] = (input_script).c_str();

            sin.str(sout.str());
            sout.str("");

            std::string&& tx_encode_index = std::to_string(i++);
            log::debug(LOG_COMMAND)<<"get_input_set:"<<tx_encode_index<<" "<<input_script;
            const char* cmds[]{"input-set", "-i", tx_encode_index.c_str(), input_script.c_str()};

            if (dispatch_command(4, cmds, sin, sout, sout))
                throw std::logic_error(sout.str());
            
        }
    }
    
    for (auto& fromeach : asset_ls_){
        std::string&& frompubkey = ec_to_xxx_impl("ec-to-public", fromeach.key);

        for (auto& iter: keys_inputs_[fromeach.key]){
            if(iter.output.as_input_sign.empty())
                continue;
            std::string&& input_script = "[ " + iter.output.as_input_sign + " ] " + "[ " + frompubkey + " ]";
            if(is_cmd_exist(bank_cmds, sizeof(bank_cmds)/sizeof(bank_cmds[0]), const_cast<char*>((input_script).c_str())))
                continue;
            bank_cmds[i] = (input_script).c_str();

            sin.str(sout.str());
            sout.str("");

            std::string&& tx_encode_index = std::to_string(i++);
            log::debug(LOG_COMMAND)<<"get_input_set:"<<tx_encode_index<<" "<<input_script;
            const char* cmds[]{"input-set", "-i", tx_encode_index.c_str(), input_script.c_str()};

            if (dispatch_command(4, cmds, sin, sout, sout))
                throw std::logic_error(sout.str());
        }
    }

    tx_set = sout.str();
    log::debug(LOG_COMMAND)<<"input-set sout:"<<tx_set;
}

uint64_t utxo_attach_send_helper::get_etp_balance()
{
    uint64_t total_balance = 0;
    for (auto& each : etp_ls_){
        total_balance += each.second;
    }
    return total_balance;
}
uint64_t utxo_attach_send_helper::get_asset_balance()
{
    uint64_t total_balance = 0;
    for (auto& each : asset_ls_){
        total_balance += each.value;
    }
    return total_balance;
}

uint64_t utxo_attach_send_helper::get_asset_amounts()
{
    uint64_t total_amount = 0;
    for (auto& each : asset_ls_){
        total_amount += each.asset_amount;
    }
    return total_amount;
}

void utxo_attach_send_helper::group_asset_amount()
{
    uint64_t k = 0;
    for (auto iter = asset_ls_.begin(); iter != asset_ls_.end(); ++iter){
        if (k > amount_){
            iter = asset_ls_.erase(iter);
            continue;
        }
        k += iter->asset_amount;
    }
}

void utxo_attach_send_helper::group_asset_etp_amount(uint64_t etp_amount)
{
    uint64_t sum_etp = 0, k = 0;
    for (auto iter = asset_ls_.begin(); iter != asset_ls_.end(); ++iter){
        if ((k > amount_) && (sum_etp > etp_amount)){
            iter = asset_ls_.erase(iter);
            continue;
        }

        k += iter->asset_amount;
        sum_etp += iter->value;
    }
}

void utxo_attach_send_helper::group_etp()
{
    uint64_t k = 0;
    for (auto iter = etp_ls_.begin(); iter != etp_ls_.end(); ++iter){
        if (k > total_payment_amount_){
            iter = etp_ls_.erase(iter);
            continue;
        }
        k += iter->second;
    }
}

bool utxo_attach_send_helper::group_utxo()
{
    auto etp_balance = get_etp_balance();
    auto asset_balance = get_asset_balance();
    auto total_amount = get_asset_amounts();
    if (((etp_balance + asset_balance) < total_payment_amount_) 
            || (total_amount < amount_))
        throw std::logic_error{"Account don't have enough balances or asset amount"};

    // 1. do etp check
    if(etp_balance < total_payment_amount_) {// etp_ls_ has not enough etp , use asset etp to fill
        group_asset_etp_amount(total_payment_amount_-etp_balance);
    } else {
        group_etp();
        group_asset_amount();
    }
    return true;
}

void utxo_attach_send_helper::get_utxo_option(utxo_attach_info& info)
{
    
    /*    
    target:1:etp                  :amount(value)  // 4
    target:1:etp-award              :amount(value)  // 4
    target:1:asset-issue   :symbol:value          // 4
    target:1:asset-transfer:symbol:amount:value   // 5
    */
    #define  PARM_SEPARATOR  ":"
    std::string ret = "";
    std::string type = info.type;
    
    if(type == "etp" || type == "etp-award" ) {
        ret = info.target + PARM_SEPARATOR 
            + std::to_string(info.version) + PARM_SEPARATOR
            + type + PARM_SEPARATOR
            + std::to_string(info.value);
    } else if(type == "asset-issue") {
        ret = info.target + PARM_SEPARATOR 
            + std::to_string(info.version) + PARM_SEPARATOR
            + type + PARM_SEPARATOR
            + info.symbol + PARM_SEPARATOR
            + std::to_string(info.value);
    } else if(type == "asset-transfer") {
        ret = info.target + PARM_SEPARATOR 
            + std::to_string(info.version) + PARM_SEPARATOR
            + type + PARM_SEPARATOR
            + info.symbol + PARM_SEPARATOR
            + std::to_string(info.amount) + PARM_SEPARATOR
            + std::to_string(info.value);
    } 

    info.output_option = ret;
}

// ----------------------------------------------------------------------------

bool send_impl(utxo_attach_send_helper& utxo, bc::blockchain::block_chain_impl& blockchain, std::ostream& output, std::ostream& cerr)
{    
    // clean from_list_ by some algorithm
    if(!utxo.group_utxo())
        throw std::logic_error{"not enough etp in account addresses"};
        //return false; // if not enough etp to pay
        
    // get utxo
    if (!utxo.fetch_utxo(blockchain))
        throw std::logic_error{"not enough etp in utxo of some address"};
        //return false;

    // load pre-transaction
    utxo.fetch_tx();

    // tx-encode
    std::string tx_encode;
    utxo.get_tx_encode(tx_encode, blockchain);

    // input-sign
    utxo.get_input_sign(tx_encode);

    // input-set
    std::string tx_set;
    utxo.get_input_set(tx_encode, tx_set);

    // tx-decode
    std::string tx_decode;
    get_tx_decode(tx_set, tx_decode);

    // validate-tx
    validate_tx(tx_set);

    // send-tx
    std::string send_ret;
    send_tx(tx_set, send_ret);

    //output<<"{\"sent-result\":\"" << send_ret <<"\",";
    output<< tx_decode ;

    return true;
}

bool utxo_attach_issuefrom_helper::fetch_utxo(bc::blockchain::block_chain_impl& blockchain)
{
    using namespace boost::property_tree;
    std::string change;
    std::string&& amount = std::to_string(total_payment_amount_);
    // find address whose all utxo balance is bigger than total_payment_amount_
    for (auto& fromeach : from_list_){

        std::string&& frompubkey = ec_to_xxx_impl("ec-to-public", fromeach.first);
        std::string&& fromaddress = ec_to_xxx_impl("ec-to-address", frompubkey, use_testnet_);

		const char* cmds[]{"xfetchutxo", amount.c_str(), fromaddress.c_str(), "-t", "etp"};
        std::ostringstream sout("");
        std::istringstream sin;
        if (dispatch_command(5, cmds, sin, sout, sout, blockchain)){
            throw std::logic_error(sout.str());
        }
        sin.str(sout.str());

        // parse json
        ptree pt; 
        read_json(sin, pt);

        change = pt.get<std::string>("change");
        auto points = pt.get_child("points");

        // not found, try next address 
        if (points.size() == 0 && change == "0"){
            //return false;
            continue;
        }

        // found, then push_back
        tx_items tx;
        for (auto& i: points){
            tx.txhash.clear();
            tx.output.index.clear();

            tx.txhash = i.second.get<std::string>("hash");
            tx.output.index  = i.second.get<std::string>("index");

            keys_inputs_[fromeach.first].push_back(tx);
        }
        //mychange_.first = fromaddress;
        //mychange_.second = std::stoull(change);
        set_mychange(fromaddress, std::stoull(change));
        return true; // found one then break

    }

    return false;
    
}


// ---------------------------------------------------------------------------
bool utxo_attach_issuefrom_helper::fetch_tx() 
{
    using namespace boost::property_tree;

    const char* cmds[]{"fetch-tx"};

    std::ostringstream sout;
    std::istringstream sin;

    for (auto& fromeach : from_list_){

        for (auto& iter : keys_inputs_[fromeach.first]){
            sout.str("");
            sin.str(iter.txhash);

            if (dispatch_command(1, cmds, sin, sout, sout))
                throw std::logic_error(sout.str());
            sin.str(sout.str());

            ptree pt;
            read_json(sin, pt);

            auto transaction = pt.get_child("transaction");
            auto outputs = transaction.get_child("outputs");

            // fill tx_items outputs
            auto target_pos = std::stoi(iter.output.index);
            int pos = 0;
            for (auto& i: outputs){
                if (target_pos == pos++){
                    iter.output.script = i.second.get<std::string>("script");
                    iter.output.value = i.second.get<uint64_t>("value");
                    break;
                }
            }
        }

    }

    return true;
}
void utxo_attach_issuefrom_helper::generate_receiver_list()
{
    if(0==business_type_.compare("etp") || 0==business_type_.compare("etp-award")) {

    } else if(0==business_type_.compare("asset-issue")) {
        //receiver_list_.push_back({mychange_.first, 1, "asset-issue", symbol_, amount_, mychange_.second, ""});
        if(amount_)
        	receiver_list_.push_back({mychange_.first, 1, "asset-issue", symbol_, amount_, 0, ""});
		if(mychange_.second)
        	receiver_list_.push_back({mychange_.first, 1, "etp", symbol_, amount_, mychange_.second, ""});
    } else if(0==business_type_.compare("asset-transfer")) {
        //auto total_amount = get_asset_amounts();
        //receiver_list_.push_back({mychange_.first, 1, "asset-transfer", symbol_, total_amount-amount_, mychange_.second, ""});
        //receiver_list_.push_back({address_, 1, "asset-transfer", symbol_, amount_, 0, ""});
    } 
}

void utxo_attach_issuefrom_helper::get_tx_encode(std::string& tx_encode, bc::blockchain::block_chain_impl& blockchain)
{
    const char* cmds[1024]{0x00};
    int i = 0;
    cmds[i++] = "encodeattachtx";
    cmds[i++] = name_.c_str();
    cmds[i++] = passwd_.c_str();

    // input args
    for (auto& fromeach : from_list_){
        for (auto& iter: keys_inputs_[fromeach.first]){ // only one address utxo inputs
            iter.output.as_tx_encode_input_args = iter.txhash + ":" + iter.output.index;
            cmds[i++] = "-i";
            cmds[i++] = iter.output.as_tx_encode_input_args.c_str();
        }
    }
    // add mychange to receiver list
    generate_receiver_list();
    
    // output args
    for (auto& iter: receiver_list_) {
        cmds[i++] = "-o";
        get_utxo_option(iter);
        cmds[i++] = iter.output_option.c_str();
    }

    std::ostringstream sout;
    std::istringstream sin;
    if (dispatch_command(i, cmds, sin, sout, sout, blockchain))
        throw std::logic_error(sout.str());

    log::debug(LOG_COMMAND)<<"encodeattachtx sout:"<<sout.str();
    tx_encode = sout.str();
}

void utxo_attach_issuefrom_helper::get_input_sign(std::string& tx_encode)
{
    std::ostringstream sout;
    std::istringstream sin;

    int i = 0;
    for (auto& fromeach : from_list_){
        for (auto& iter: keys_inputs_[fromeach.first]){
            sin.str(tx_encode);
            sout.str("");

            std::string&& tx_encode_index = std::to_string(i++);
            const char* cmds[]{"input-sign", "-i", tx_encode_index.c_str(), fromeach.first.c_str(), iter.output.script.c_str()};
            if (dispatch_command(5, cmds, sin, sout, sout))
                throw std::logic_error(sout.str());
            iter.output.as_input_sign = sout.str();
            log::debug(LOG_COMMAND)<<"input-sign sout:"<<iter.output.as_input_sign;
        }
    }
}

void utxo_attach_issuefrom_helper::get_input_set(const std::string& tx_encode, std::string& tx_set)
{
    std::ostringstream sout(tx_encode);
    std::istringstream sin;

    int i = 0;
    for (auto& fromeach : from_list_){
        std::string&& frompubkey = ec_to_xxx_impl("ec-to-public", fromeach.first);

        for (auto& iter: keys_inputs_[fromeach.first]){
            std::string&& input_script = "[ " + iter.output.as_input_sign + " ] " + "[ " + frompubkey + " ]";
            sin.str(sout.str());
            sout.str("");

            std::string&& tx_encode_index = std::to_string(i++);
            const char* cmds[]{"input-set", "-i", tx_encode_index.c_str(), input_script.c_str()};

            if (dispatch_command(4, cmds, sin, sout, sout))
                throw std::logic_error(sout.str());
        }
    }

    tx_set = sout.str();
    log::debug(LOG_COMMAND)<<"input-set sout:"<<tx_set;
}

uint64_t utxo_attach_issuefrom_helper::get_my_balance()
{
    uint64_t total_balance = 0;
    for (auto& each : from_list_){
        total_balance += each.second;
    }
    return total_balance;
}
void utxo_attach_issuefrom_helper::group_utxo()
{
    auto balance = get_my_balance();
    if (balance < total_payment_amount_)
        throw std::logic_error{"Account don't have enough balances"};
}

void utxo_attach_issuefrom_helper::get_utxo_option(utxo_attach_info& info)
{
    
    /*    
    target:1:etp                  :amount(value)  // 4
    target:1:etp-award              :amount(value)  // 4
    target:1:asset-issue   :symbol:value          // 4
    target:1:asset-transfer:symbol:amount:value   // 5
    */
    #define  PARM_SEPARATOR  ":"
    std::string ret = "";
    std::string type = info.type;
    
    if(type == "etp" || type == "etp-award" ) {
        ret = info.target + PARM_SEPARATOR 
            + std::to_string(info.version) + PARM_SEPARATOR
            + type + PARM_SEPARATOR
            + std::to_string(info.value);
    } else if(type == "asset-issue") {
        ret = info.target + PARM_SEPARATOR 
            + std::to_string(info.version) + PARM_SEPARATOR
            + type + PARM_SEPARATOR
            + info.symbol + PARM_SEPARATOR
            + std::to_string(info.value);
    } else if(type == "asset-transfer") {
        ret = info.target + PARM_SEPARATOR 
            + std::to_string(info.version) + PARM_SEPARATOR
            + type + PARM_SEPARATOR
            + info.symbol + PARM_SEPARATOR
            + std::to_string(info.amount) + PARM_SEPARATOR
            + std::to_string(info.value);
    } 

    info.output_option = ret;
}

bool send_impl(utxo_attach_issuefrom_helper& utxo, bc::blockchain::block_chain_impl& blockchain, 
    std::ostream& output, std::ostream& cerr)
{
    // clean from_list_ by some algorithm
    utxo.group_utxo();

    // get utxo
    if (!utxo.fetch_utxo(blockchain))
        throw std::logic_error{"not enough etp in utxo of some address"};

    // load pre-transaction
    utxo.fetch_tx();

    // tx-encode
    std::string tx_encode;
    utxo.get_tx_encode(tx_encode, blockchain);

    // input-sign
    utxo.get_input_sign(tx_encode);

    // input-set
    std::string tx_set;
    utxo.get_input_set(tx_encode, tx_set);

    // tx-decode
    std::string tx_decode;
    get_tx_decode(tx_set, tx_decode);

    // validate-tx
    validate_tx(tx_set);

    // send-tx
    std::string send_ret;
    send_tx(tx_set, send_ret);

    //output<<"{\"sent-result\":\"" << send_ret <<"\",";
    output<< tx_decode ;

    return true;
}

/*************************************** sendassetfrom *****************************************/
bool utxo_attach_sendfrom_helper::fetch_utxo_impl(bc::blockchain::block_chain_impl& blockchain,
    std::string& prv_key, uint64_t payment_amount, uint64_t& utxo_change)
{
    using namespace boost::property_tree;

    std::string&& amount = std::to_string(payment_amount);
    std::string&& frompubkey = ec_to_xxx_impl("ec-to-public", prv_key);
    std::string&& fromaddress = ec_to_xxx_impl("ec-to-address", frompubkey, use_testnet_);

    const char* cmds[]{"xfetchutxo", amount.c_str(), fromaddress.c_str()};

    std::ostringstream sout("");
    std::istringstream sin;
    if (dispatch_command(3, cmds, sin, sout, sout, blockchain)){
        throw std::logic_error(sout.str());
    }
    sin.str(sout.str());

    // parse json
    ptree pt; 
    read_json(sin, pt);

    std::string change = pt.get<std::string>("change");
    auto points = pt.get_child("points");

    // not found, return  
    if (points.size() == 0 && change == "0"){
            return false;
    }

    // found, then push_back
    tx_items tx;
    for (auto& i: points){
        tx.txhash.clear();
        tx.output.index.clear();

        tx.txhash = i.second.get<std::string>("hash");
        tx.output.index  = i.second.get<std::string>("index");
        #if 0
        bool exist = false;
        for(auto& item: keys_inputs_[prv_key]) {
            if((item.txhash == tx.txhash) && (item.output.index == tx.output.index))
                exist = true;
        }
        
        if(!exist)
        #endif
        keys_inputs_[prv_key].push_back(tx);
    }
    utxo_change = std::stoull(change);
    
    return true;
    
}

bool utxo_attach_sendfrom_helper::fetch_utxo()
{
    using namespace libbitcoin::config; // for hash256
    uint64_t asset_amount = 0, etp_num = 0;
    
    for (auto& each : asset_ls_){ 
        //fetch_utxo_impl(fromeach.key, 0, change);
        //utxo_change += change;
        // todo -- maybe following code added later
        //if((etp_num >= total_payment_amount_) && (asset_amount >= amount_))
            //break;
        tx_items tx;
        tx.txhash.clear();
        tx.output.index.clear();
    
        tx.txhash = hash256(each.output.hash).to_string();
        tx.output.index  = std::to_string(each.output.index);
        keys_inputs_[each.key].push_back(tx);
        
        // accumualte etp and asset amount
        etp_num += each.value;
        asset_amount += each.asset_amount;
    }
    
    // fill reback change and asset amount
    std::string&& frompubkey = ec_to_xxx_impl("ec-to-public", asset_ls_.begin()->key);
    std::string&& fromaddress = ec_to_xxx_impl("ec-to-address", frompubkey, use_testnet_);
    set_mychange(fromaddress, etp_num - total_payment_amount_, asset_amount - amount_);
    
    return true; 
}

// ---------------------------------------------------------------------------
bool utxo_attach_sendfrom_helper::fetch_tx() 
{
    using namespace boost::property_tree;

    const char* cmds[]{"fetch-tx"};

    std::ostringstream sout;
    std::istringstream sin;
    
    for (auto& fromeach : prikey_set_){

        for (auto& iter : keys_inputs_[fromeach]){
            sout.str("");
            sin.str(iter.txhash);

            if (dispatch_command(1, cmds, sin, sout, sout))
                throw std::logic_error(sout.str());
            sin.str(sout.str());

            ptree pt;
            read_json(sin, pt);

            auto transaction = pt.get_child("transaction");
            auto outputs = transaction.get_child("outputs");

            // fill tx_items outputs
            auto target_pos = std::stoi(iter.output.index);
            int pos = 0;
            for (auto& i: outputs){
                if (target_pos == pos++){
                    iter.output.script = i.second.get<std::string>("script");
                    iter.output.value = i.second.get<uint64_t>("value");
                    break;
                }
            }
        }

    }

    return true;
}
void utxo_attach_sendfrom_helper::generate_receiver_list()
{
    if(0==business_type_.compare("etp") || 0==business_type_.compare("etp-award")) {

    } else if(0==business_type_.compare("asset-issue")) {
        receiver_list_.push_back({mychange_.addr, 1, "asset-transfer", symbol_, amount_, mychange_.etp_value, ""});
    } else if(0==business_type_.compare("asset-transfer")) {
        //auto total_amount = get_asset_amounts();
		if(mychange_.asset_amount)
            receiver_list_.push_back({mychange_.addr, 1, "asset-transfer", symbol_, mychange_.asset_amount, 0, ""});
		if(mychange_.etp_value)
            receiver_list_.push_back({mychange_.addr, 1, "etp", "", 0, mychange_.etp_value, ""});
        // target asset receiver
        if(amount_)
        	receiver_list_.push_back({address_, 1, "asset-transfer", symbol_, amount_, 0, ""});
    } 
}
void utxo_attach_sendfrom_helper::get_tx_encode(std::string& tx_encode, bc::blockchain::block_chain_impl& blockchain)
{
    const char* cmds[1024]{0x00};
    int i = 0;
    cmds[i++] = "encodeattachtx";
    cmds[i++] = name_.c_str();
    cmds[i++] = passwd_.c_str();

    // input args
    for (auto& fromeach : prikey_set_){
        for (auto& iter: keys_inputs_[fromeach]){ // only one address utxo inputs
            iter.output.as_tx_encode_input_args = iter.txhash + ":" + iter.output.index;
            //if(is_cmd_exist(cmds, sizeof(cmds)/sizeof(cmds[0]), const_cast<char*>(iter.output.as_tx_encode_input_args.c_str())))
                //continue;
            cmds[i++] = "-i";
            cmds[i++] = iter.output.as_tx_encode_input_args.c_str();
        }
    }
    
    // generate receiver list according from list
    generate_receiver_list();
    
    // output args
    for (auto& iter: receiver_list_) {
        cmds[i++] = "-o";
        get_utxo_option(iter);
        cmds[i++] = iter.output_option.c_str();
    }

    std::ostringstream sout;
    std::istringstream sin;
    if (dispatch_command(i, cmds, sin, sout, sout, blockchain))
        throw std::logic_error(sout.str());

    log::debug(LOG_COMMAND)<<"encodeattachtx sout:"<<sout.str();
    tx_encode = sout.str();
}

void utxo_attach_sendfrom_helper::get_input_sign(std::string& tx_encode)
{
    std::ostringstream sout;
    std::istringstream sin;
    
    //const char* bank_cmds[1024]{0x00};

    int i = 0;    
    for (auto& fromeach : prikey_set_){
        for (auto& iter: keys_inputs_[fromeach]){
            //if(!iter.output.as_input_sign.empty())  // only assign once
                //continue;
            sin.str(tx_encode);
            sout.str("");

            std::string&& tx_encode_index = std::to_string(i++);
            
            //if(is_cmd_exist(bank_cmds, sizeof(bank_cmds)/sizeof(bank_cmds[0]), const_cast<char*>((tx_encode_index + fromeach.key + iter.output.script).c_str())))
                //continue;
            //bank_cmds[i] = ((tx_encode_index + fromeach.key + iter.output.script).c_str());
            const char* cmds[]{"input-sign", "-i", tx_encode_index.c_str(), fromeach.c_str(), iter.output.script.c_str()};
            log::debug(LOG_COMMAND)<<"asset input-sign="<<tx_encode_index<<" "<<fromeach<<" "<<iter.output.script;
            if (dispatch_command(5, cmds, sin, sout, sout))
                throw std::logic_error(sout.str());
            iter.output.as_input_sign = sout.str();
            log::debug(LOG_COMMAND)<<"input-sign sout:"<<iter.output.as_input_sign;
        }
    }
}

void utxo_attach_sendfrom_helper::get_input_set(const std::string& tx_encode, std::string& tx_set)
{
    std::ostringstream sout(tx_encode);
    std::istringstream sin;
    //const char* bank_cmds[1024]{0x00};

    int i = 0;
    
    for (auto& fromeach : prikey_set_){
        std::string&& frompubkey = ec_to_xxx_impl("ec-to-public", fromeach);

        for (auto& iter: keys_inputs_[fromeach]){
            //if(iter.output.as_input_sign.empty())
                //continue;
            std::string&& input_script = "[ " + iter.output.as_input_sign + " ] " + "[ " + frompubkey + " ]";
            //if(is_cmd_exist(bank_cmds, sizeof(bank_cmds)/sizeof(bank_cmds[0]), const_cast<char*>((input_script).c_str())))
                //continue;
            //bank_cmds[i] = (input_script).c_str();

            sin.str(sout.str());
            sout.str("");

            std::string&& tx_encode_index = std::to_string(i++);
            log::debug(LOG_COMMAND)<<"get_input_set:"<<tx_encode_index<<" "<<input_script;
            const char* cmds[]{"input-set", "-i", tx_encode_index.c_str(), input_script.c_str()};

            if (dispatch_command(4, cmds, sin, sout, sout))
                throw std::logic_error(sout.str());
        }
    }

    tx_set = sout.str();
    log::debug(LOG_COMMAND)<<"input-set sout:"<<tx_set;
}

uint64_t utxo_attach_sendfrom_helper::get_asset_balance()
{
    uint64_t total_balance = 0;
    for (auto& each : asset_ls_){
        total_balance += each.value;
    }
    return total_balance;
}

uint64_t utxo_attach_sendfrom_helper::get_asset_amounts()
{
    uint64_t total_amount = 0;
    for (auto& each : asset_ls_){
        total_amount += each.asset_amount;
    }
    return total_amount;
}

void utxo_attach_sendfrom_helper::group_utxo()
{
    auto asset_balance = get_asset_balance();
    auto total_amount = get_asset_amounts();
    if ((asset_balance < total_payment_amount_) 
            || (total_amount < amount_))
        throw std::logic_error{"Account don't have enough balances or asset amount"};
}

void utxo_attach_sendfrom_helper::get_utxo_option(utxo_attach_info& info)
{
    
    /*    
    target:1:etp                  :amount(value)  // 4
    target:1:etp-award              :amount(value)  // 4
    target:1:asset-issue   :symbol:value          // 4
    target:1:asset-transfer:symbol:amount:value   // 5
    */
    #define  PARM_SEPARATOR  ":"
    std::string ret = "";
    std::string type = info.type;
    
    if(type == "etp" || type == "etp-award" ) {
        ret = info.target + PARM_SEPARATOR 
            + std::to_string(info.version) + PARM_SEPARATOR
            + type + PARM_SEPARATOR
            + std::to_string(info.value);
    } else if(type == "asset-issue") {
        ret = info.target + PARM_SEPARATOR 
            + std::to_string(info.version) + PARM_SEPARATOR
            + type + PARM_SEPARATOR
            + info.symbol + PARM_SEPARATOR
            + std::to_string(info.value);
    } else if(type == "asset-transfer") {
        ret = info.target + PARM_SEPARATOR 
            + std::to_string(info.version) + PARM_SEPARATOR
            + type + PARM_SEPARATOR
            + info.symbol + PARM_SEPARATOR
            + std::to_string(info.amount) + PARM_SEPARATOR
            + std::to_string(info.value);
    } 

    info.output_option = ret;
}


// ----------------------------------------------------------------------------

bool send_impl(utxo_attach_sendfrom_helper& utxo, bc::blockchain::block_chain_impl& blockchain, std::ostream& output, std::ostream& cerr)
{    
    // clean from_list_ by some algorithm
    utxo.group_utxo();
        
    // get utxo
    if (!utxo.fetch_utxo())
        throw std::logic_error{"not enough etp in utxo of some address"};

    // load pre-transaction
    utxo.fetch_tx();

    // tx-encode
    std::string tx_encode;
    utxo.get_tx_encode(tx_encode, blockchain);

    // input-sign
    utxo.get_input_sign(tx_encode);

    // input-set
    std::string tx_set;
    utxo.get_input_set(tx_encode, tx_set);

    // tx-decode
    std::string tx_decode;
    get_tx_decode(tx_set, tx_decode);

    // validate-tx
    validate_tx(tx_set);

    // send-tx
    std::string send_ret;
    send_tx(tx_set, send_ret);

    //output<<"{\"sent-result\":\"" << send_ret <<"\",";
    output<< tx_decode ;

    return true;
}

}// commands
}// explorer
}// libbitcoin
