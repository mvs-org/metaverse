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

#include <metaverse/explorer/extensions/command_assistant.hpp>
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
        total_payment_amount += std::stoull(results[1], nullptr, 10);
    }

    // last one for mychange and fee
    auto fee = std::stoull(results[1], nullptr, 10);
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
std::string get_multisig_script(uint8_t m, uint8_t n, std::vector<std::string>& public_keys){
	std::sort(public_keys.begin(), public_keys.end());
	std::ostringstream ss;
	ss << std::to_string(m);
	for(auto& each : public_keys)
		ss << " [ " << each << " ] ";
	ss << std::to_string(n) << " checkmultisig";
	return ss.str();
}
void get_multisig_pri_pub_key(std::string& prikey, std::string& pubkey, std::string& seed, uint32_t hd_index){
	const char* cmds[]{"mnemonic-to-seed", "hd-new", "hd-to-ec", "ec-to-public", "ec-to-address"};
	
	//data_chunk data(seed.begin(), seed.end());
	//std::ostringstream sout(encode_hash(bitcoin_hash(data)));
	std::ostringstream sout(seed);
	std::istringstream sin("");
	
	auto exec_with = [&](int i){
		sin.str(sout.str());
		sout.str("");
		return dispatch_command(1, cmds + i, sin, sout, sout);
	};
			
	exec_with(1); // hd-new
	
	auto&& argv_index = std::to_string(hd_index);
	const char* hd_private_gen[3] = {"hd-private", "-i", argv_index.c_str()};
	sin.str(sout.str());
	sout.str("");
	dispatch_command(3, hd_private_gen, sin, sout, sout); // hd-private
	
	exec_with(2); // hd-to-ec
	prikey = sout.str();
	//acc->set_multisig_prikey(multisig_prikey);
	//root.put("multisig-prikey", sout.str());
	//addr->set_prv_key(sout.str(), auth_.auth);
	// not store public key now
	exec_with(3); // ec-to-public
	//addr->set_pub_key(sout.str());
	pubkey = sout.str();
}
history::list expand_history(history_compact::list& compact)
{
    history::list result;

    // Process and remove all outputs.
    for (auto output = compact.begin(); output != compact.end();)
    {
        if (output->kind == point_kind::output)
        {
            history row;
            row.output = output->point;
            row.output_height = output->height;
            row.value = output->value;
            row.spend = { null_hash, max_uint32 };
            row.temporary_checksum = output->point.checksum();
            result.emplace_back(row);
            output = compact.erase(output);
            continue;
        }

        ++output;
    }

    // All outputs have been removed, process the spends.
    for (const auto& spend: compact)
    {
        auto found = false;

        // Update outputs with the corresponding spends.
        for (auto& row: result)
        {
            if (row.temporary_checksum == spend.previous_checksum &&
                row.spend.hash == null_hash)
            {
                row.spend = spend.point;
                row.spend_height = spend.height;
                found = true;
                break;
            }
        }

        // This will only happen if the history height cutoff comes between
        // an output and its spend. In this case we return just the spend.
        if (!found)
        {
            history row;
            row.output = { null_hash, max_uint32 };
            row.output_height = max_uint64;
            row.value = max_uint64;
            row.spend = spend.point;
            row.spend_height = spend.height;
            result.emplace_back(row);
        }
    }

    compact.clear();

    // Clear all remaining checksums from unspent rows.
    for (auto& row: result)
        if (row.spend.hash == null_hash)
            row.spend_height = max_uint64;

    // TODO: sort by height and index of output, spend or both in order.
    return result;
}
history::list get_address_history(wallet::payment_address& addr, bc::blockchain::block_chain_impl& blockchain)
{
	history_compact::list cmp_history;
	history::list history_vec;
	if(blockchain.get_history(addr, 0, 0, cmp_history)) {
		
		history_vec = expand_history(cmp_history);
	}
	return history_vec;
}

void expand_history(history_compact::list& compact, history::list& result)
{
    // Process and remove all outputs.
    for (auto output = compact.begin(); output != compact.end();)
    {
        if (output->kind == point_kind::output)
        {
            history row;
            row.output = output->point;
            row.output_height = output->height;
            row.value = output->value;
            row.spend = { null_hash, max_uint32 };
            row.temporary_checksum = output->point.checksum();
            result.emplace_back(row);
            output = compact.erase(output);
            continue;
        }

        ++output;
    }

    // All outputs have been removed, process the spends.
    for (const auto& spend: compact)
    {
        auto found = false;

        // Update outputs with the corresponding spends.
        for (auto& row: result)
        {
            if (row.temporary_checksum == spend.previous_checksum &&
                row.spend.hash == null_hash)
            {
                row.spend = spend.point;
                row.spend_height = spend.height;
                found = true;
                break;
            }
        }

        // This will only happen if the history height cutoff comes between
        // an output and its spend. In this case we return just the spend.
        if (!found)
        {
            history row;
            row.output = { null_hash, max_uint32 };
            row.output_height = max_uint64;
            row.value = max_uint64;
            row.spend = spend.point;
            row.spend_height = spend.height;
            result.emplace_back(row);
        }
    }

    compact.clear();

    // Clear all remaining checksums from unspent rows.
    for (auto& row: result)
        if (row.spend.hash == null_hash)
            row.spend_height = max_uint64;
}

void get_address_history(wallet::payment_address& addr, bc::blockchain::block_chain_impl& blockchain,
	history::list& history_vec)
{
	history_compact::list cmp_history;
	if(blockchain.get_history(addr, 0, 0, cmp_history)) {
		
		expand_history(cmp_history, history_vec);
	}
}
// for xfetchutxo command
chain::points_info sync_fetchutxo(uint64_t amount, wallet::payment_address& addr, 
	std::string& type, bc::blockchain::block_chain_impl& blockchain)
{
	// history::list rows
	auto rows = get_address_history(addr, blockchain);
	log::trace("get_history=")<<rows.size();
	
	uint64_t height = 0;
	blockchain.get_last_height(height);
	chain::output_info::list unspent;
	chain::transaction tx_temp;
	uint64_t tx_height;
	uint64_t total_unspent = 0;
	
	for (auto& row: rows)
	{		
		// spend unconfirmed (or no spend attempted)
		if (row.spend.hash == null_hash
				&& blockchain.get_transaction(row.output.hash, tx_temp, tx_height)) {
			// fetch utxo script to check deposit utxo
			//log::debug("get_tx=")<<blockchain.get_transaction(row.output.hash, tx_temp); // todo -- return value check
			auto output = tx_temp.outputs.at(row.output.index);
			bool is_deposit_utxo = false;

			// deposit utxo in transaction pool
			if ((output.script.pattern() == bc::chain::script_pattern::pay_key_hash_with_lock_height)
						&& !row.output_height) { 
				is_deposit_utxo = true;
		    }

			// deposit utxo in block
			if(chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations)
				&& row.output_height) { 
				uint64_t lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(output.script.operations);
				if((row.output_height + lock_height) > height) { // utxo already in block but deposit not expire
					is_deposit_utxo = true;
				}
			}
			
			// coin base etp maturity etp check
			if(tx_temp.is_coinbase()
				&& !(output.script.pattern() == bc::chain::script_pattern::pay_key_hash_with_lock_height)) { // incase readd deposit
				// add not coinbase_maturity etp into frozen
				if((!row.output_height ||
							(row.output_height && (height - row.output_height) < coinbase_maturity))) {
					is_deposit_utxo = true; // not deposit utxo but can not spent now
				}
			}
			
			if(is_deposit_utxo)
				continue;
			
			if((type == "all") 
				|| ((type == "etp") && output.is_etp())){
				total_unspent += row.value;
				unspent.push_back({row.output, row.value});
			}
		}
		// algorithm optimize
		if(total_unspent >= amount)
			break;
	
	}
	
	chain::points_info selected_utxos;
	wallet::select_outputs::select(selected_utxos, unspent, amount);

	return selected_utxos;

}
/// amount == 0 -- get all address balances
/// amount != 0 -- get some address balances which bigger than amount 
void sync_fetchbalance (wallet::payment_address& address, 
	std::string& type, bc::blockchain::block_chain_impl& blockchain, balances& addr_balance, uint64_t amount)
{
	// history::list rows
	auto rows = get_address_history(address, blockchain);
	log::trace("get_history=")<<rows.size();

	uint64_t total_received = 0;
	uint64_t confirmed_balance = 0;
	uint64_t unspent_balance = 0;
	uint64_t frozen_balance = 0;
	
	chain::transaction tx_temp;
	uint64_t tx_height;
	uint64_t height = 0;
	blockchain.get_last_height(height);

	for (auto& row: rows)
	{
		if(amount && ((unspent_balance - frozen_balance) >= amount)) // performance improve
			break;
		
		total_received += row.value;
	
		// spend unconfirmed (or no spend attempted)
		if ((row.spend.hash == null_hash)
				&& blockchain.get_transaction(row.output.hash, tx_temp, tx_height)) {
			//log::debug("get_tx=")<<blockchain.get_transaction(row.output.hash, tx_temp); // todo -- return value check
			auto output = tx_temp.outputs.at(row.output.index);

			// deposit utxo in transaction pool
			if ((output.script.pattern() == bc::chain::script_pattern::pay_key_hash_with_lock_height)
						&& !row.output_height) { 
				frozen_balance += row.value;
		    }

			// deposit utxo in block
			if(chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations)
				&& row.output_height) { 
				uint64_t lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(output.script.operations);
				if((row.output_height + lock_height) > height) { // utxo already in block but deposit not expire
					frozen_balance += row.value;
				}
			}
			
			// coin base etp maturity etp check
			if(tx_temp.is_coinbase()
				&& !(output.script.pattern() == bc::chain::script_pattern::pay_key_hash_with_lock_height)) { // incase readd deposit
				// add not coinbase_maturity etp into frozen
				if((!row.output_height ||
							(row.output_height && (height - row.output_height) < coinbase_maturity))) {
					frozen_balance += row.value;
				}
			}
			
			if((type == "all") 
				|| ((type == "etp") && output.is_etp()))
				unspent_balance += row.value;
		}
	
		if (row.output_height != 0 &&
			(row.spend.hash == null_hash || row.spend_height == 0))
			confirmed_balance += row.value;
	}
	
	addr_balance.confirmed_balance = confirmed_balance;
	addr_balance.total_received = total_received;
	addr_balance.unspent_balance = unspent_balance;
	addr_balance.frozen_balance = frozen_balance;
	
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

    // mychange == 0
    if (std::stoull(mychange) == 0)
        return;

    receiver_list_.push_back({mychange_.first + ":" + mychange});

#if 0 //there are some bug when deposit
    // random sort receiver_list_
    auto random = bc::pseudo_random();
    std::sort(receiver_list_.begin(), receiver_list_.end(), 
            [random](const std::string& s1, const std::string& s2){
                return random%2 == 0;
            });
#endif

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
void base_transfer_helper::sum_payment_amount(){
	if(receiver_list_.empty())
		throw std::logic_error{"empty target address"};
    if (payment_etp_ > maximum_fee || payment_etp_ < minimum_fee)
        throw std::logic_error{"fee must in [10000, 10000000000]"};

    for (auto& iter : receiver_list_) {
        payment_etp_ += iter.amount;
        payment_asset_ += iter.asset_amount;
    }
}
void base_transfer_helper::sync_fetchutxo (const std::string& prikey, const std::string& addr, uint32_t hd_index) {
	auto waddr = wallet::payment_address(addr);
	// history::list rows
	auto rows = get_address_history(waddr, blockchain_);
	log::trace("get_history=")<<rows.size();
		
	chain::transaction tx_temp;
	uint64_t tx_height;
	uint64_t height = 0;
	auto frozen_flag = false;
	address_asset_record record;
	
	blockchain_.get_last_height(height);

	for (auto& row: rows)
	{
		frozen_flag = false;
		if((unspent_etp_ >= payment_etp_) && (unspent_asset_ >= payment_asset_)) // performance improve
			break;
				
		// spend unconfirmed (or no spend attempted)
		if ((row.spend.hash == null_hash)
				&& blockchain_.get_transaction(row.output.hash, tx_temp, tx_height)) {
			auto output = tx_temp.outputs.at(row.output.index);

			// deposit utxo in transaction pool
			if ((output.script.pattern() == bc::chain::script_pattern::pay_key_hash_with_lock_height)
						&& !row.output_height) { 
				frozen_flag = true;
			}

			// deposit utxo in block
			if(chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations)
				&& row.output_height) { 
				uint64_t lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(output.script.operations);
				if((row.output_height + lock_height) > height) { // utxo already in block but deposit not expire
					frozen_flag = true;
				}
			}
			
			// coin base etp maturity etp check
			if(tx_temp.is_coinbase()
				&& !(output.script.pattern() == bc::chain::script_pattern::pay_key_hash_with_lock_height)) { // incase readd deposit
				// add not coinbase_maturity etp into frozen
				if((!row.output_height ||
							(row.output_height && (height - row.output_height) < coinbase_maturity))) {
					frozen_flag = true;
				}
			}
			log::trace("frozen_flag=")<< frozen_flag;
			log::trace("payment_asset_=")<< payment_asset_;
			log::trace("is_etp=")<< output.is_etp();
			log::trace("value=")<< row.value;
			log::trace("is_trans=")<< output.is_asset_transfer();
			log::trace("is_issue=")<< output.is_asset_issue();
			log::trace("symbol=")<< symbol_;
			log::trace("outpuy symbol=")<< output.get_asset_symbol();
			// add to from list
			if(!frozen_flag){
				// etp -> etp tx
				if(!payment_asset_ && output.is_etp()){
					record.prikey = prikey;
					record.addr = addr;
					record.hd_index = hd_index;
					record.amount = row.value;
					record.symbol = "";
					record.asset_amount = 0;
					record.status = asset::asset_status::asset_locked;
					record.output = row.output;
					record.script = output.script;
					
					if(unspent_etp_ < payment_etp_) {
						from_list_.push_back(record);
						unspent_etp_ += record.amount;
					}
				// asset issue/transfer
				} else { 
					if(output.is_etp()){
						record.prikey = prikey;
						record.addr = addr;
						record.hd_index = hd_index;
						record.amount = row.value;
						record.symbol = "";
						record.asset_amount = 0;
						record.status = asset::asset_status::asset_none;
						record.output = row.output;
						record.script = output.script;
						
						if(unspent_etp_ < payment_etp_) {
							from_list_.push_back(record);
							unspent_etp_ += record.amount;
						}
					} else if (output.is_asset_issue() && (symbol_ == output.get_asset_symbol())){
						record.prikey = prikey;
						record.addr = addr;
						record.hd_index = hd_index;
						record.amount = row.value;
						record.symbol = output.get_asset_symbol();
						record.asset_amount = output.get_asset_amount();
						record.status = asset::asset_status::asset_locked;
						record.output = row.output;
						record.script = output.script;
						
						if((unspent_asset_ < payment_asset_)
							|| (unspent_etp_ < payment_etp_)) {
							from_list_.push_back(record);
							unspent_asset_ += record.asset_amount;
							unspent_etp_ += record.amount;
						}
					} else if (output.is_asset_transfer() && (symbol_ == output.get_asset_symbol())){
						record.prikey = prikey;
						record.addr = addr;
						record.hd_index = hd_index;
						record.amount = row.value;
						record.symbol = output.get_asset_symbol();
						record.asset_amount = output.get_asset_amount();
						record.status = asset::asset_status::asset_transferable;
						record.output = row.output;
						record.script = output.script;
						
						if((unspent_asset_ < payment_asset_)
							|| (unspent_etp_ < payment_etp_)){
							from_list_.push_back(record);
							unspent_asset_ += record.asset_amount;
							unspent_etp_ += record.amount;
						}
						log::trace("unspent_asset_=")<< unspent_asset_;
						log::trace("unspent_etp_=")<< unspent_etp_;
					}
				}
			}
		}
	
	}
	
}

void base_transfer_helper::populate_unspent_list() {
	// get address list
	auto pvaddr = blockchain_.get_account_addresses(name_);
	if(!pvaddr) 
		throw std::logic_error{"nullptr for address list"};

	// get from address balances
	for (auto& each : *pvaddr){
		if(from_.empty()) { // select utxo in all account addresses
			base_transfer_helper::sync_fetchutxo (each.get_prv_key(passwd_), each.get_address(), each.get_hd_index());
			if((unspent_etp_ >= payment_etp_)
				&& (unspent_asset_ >= payment_asset_))
				break;
		} else { // select utxo only in from_ address
			if ( from_ == each.get_address() ) { // find address
				base_transfer_helper::sync_fetchutxo (each.get_prv_key(passwd_), each.get_address(), each.get_hd_index());
				if((unspent_etp_ >= payment_etp_)
					&& (unspent_asset_ >= payment_asset_))
					break;
			}
		}
	}
	
	if(from_list_.empty())
		throw std::logic_error{"not enough etp in from address or you are't own from address!"};

	// addresses balances check
	if(unspent_etp_ < payment_etp_)
		throw std::logic_error{"no enough balance"};
	if(unspent_asset_ < payment_asset_)
		throw std::logic_error{"no enough asset amount"};

	// change
	populate_change();
}

void base_transfer_helper::populate_tx_inputs(){
    // input args
    uint64_t adjust_amount = 0;
	tx_input_type input;

    for (auto& fromeach : from_list_){
        adjust_amount += fromeach.amount;
        if (tx_item_idx_ >= tx_limit) // limit in ~333 inputs
        {
            auto&& response = "Too many inputs limit, suggest less than " + std::to_string(adjust_amount) + " satoshi.";
            throw std::runtime_error(response);
        }
		tx_item_idx_++;
		input.sequence = max_input_sequence;
		input.previous_output.hash = fromeach.output.hash;
		input.previous_output.index = fromeach.output.index;
		tx_.inputs.push_back(input);
    }
}
attachment base_transfer_helper::populate_output_attachment(receiver_record& record){
			
	if(record.symbol.empty()
		|| ((record.amount > 0) && (!record.asset_amount))) { // etp
		return attachment(ETP_TYPE, attach_version, etp(record.amount));
	} 

	if(record.status == asset::asset_status::asset_locked) {
		//std::shared_ptr<std::vector<business_address_asset>>
		auto sh_asset = blockchain_.get_account_asset(name_, symbol_);
		if(sh_asset->empty())
			throw std::logic_error{symbol_ + " not found"};
		//if(sh_asset->at(0).detail.get_maximum_supply() != record.asset_amount)
			//throw std::logic_error{symbol_ + " amount not match with maximum supply"};
		
		sh_asset->at(0).detail.set_address(record.target); // target is setted in metaverse_output.cpp
		auto ass = asset(ASSET_DETAIL_TYPE, sh_asset->at(0).detail);
		return attachment(ASSET_TYPE, attach_version, ass);
	} else if(record.status == asset::asset_status::asset_transferable) {
		auto transfer = asset_transfer(record.symbol, record.asset_amount);
		auto ass = asset(ASSET_TRANSFERABLE_TYPE, transfer);
		return attachment(ASSET_TYPE, attach_version, ass);
	} 

	throw std::logic_error{"invalid attachment value in receiver_record"};
}

void base_transfer_helper::populate_tx_outputs(){
	chain::operation::stack payment_ops;
	
    for (auto& iter: receiver_list_) {
        if (tx_item_idx_ >= (tx_limit + 10)) {
                throw std::runtime_error{"Too many inputs/outputs makes tx too large, canceled."};
        }
		tx_item_idx_++;
		
		// filter zero etp and asset. status check just for issue asset
		if( !iter.amount && !iter.asset_amount && (iter.status != asset::asset_status::asset_locked))
			continue;
		
		// complicated script and asset should be implemented in subclass
		// generate script			
		const wallet::payment_address payment(iter.target);
		if (!payment)
			throw std::logic_error{"invalid target address"};
		auto hash = payment.hash();
		if((payment.version() == 0x7f) // test net addr
			|| (payment.version() == 0x32)) { // main net addr
			payment_ops = chain::operation::to_pay_key_hash_pattern(hash); // common payment script
		} else if(payment.version() == 0x5) { // pay to script addr
			payment_ops = chain::operation::to_pay_script_hash_pattern(hash); // common payment script
		} else {
			throw std::logic_error{"unrecognized target address."};
		}
		auto payment_script = chain::script{ payment_ops };
		
		// generate asset info
		auto output_att = populate_output_attachment(iter);
		
		// fill output
		tx_.outputs.push_back({ iter.amount, payment_script, output_att });
    }
}

void base_transfer_helper::check_tx(){
    if (tx_.is_locktime_conflict())
    {
        throw std::logic_error{"The specified lock time is ineffective because all sequences are set to the maximum value."};
    }
}

void base_transfer_helper::sign_tx_inputs(){
    uint32_t index = 0;
    for (auto& fromeach : from_list_){
        // paramaters
        explorer::config::hashtype sign_type;
        uint8_t hash_type = (signature_hash_algorithm)sign_type;

        bc::explorer::config::ec_private config_private_key(fromeach.prikey);
        const ec_secret& private_key =    config_private_key;    
        bc::wallet::ec_private ec_private_key(private_key, 0u, true);

        bc::explorer::config::script config_contract(fromeach.script);
        const bc::chain::script& contract = config_contract;

        // gen sign
        bc::endorsement endorse;
        if (!bc::chain::script::create_endorsement(endorse, private_key,
            contract, tx_, index, hash_type))
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
        if (contract.pattern() == bc::chain::script_pattern::pay_key_hash_with_lock_height) {
        	uint64_t lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(
                contract.operations);
            ss.operations.push_back({bc::chain::opcode::special, satoshi_to_chunk(lock_height)});
        }
        // set input script of this tx
        tx_.inputs[index].script = ss;
        index++;
    }

}

void base_transfer_helper::send_tx(){
	if(!blockchain_.broadcast_transaction(tx_)) 
			throw std::logic_error{"broadcast_transaction failure"};
}
void base_transfer_helper::exec(){	
	// prepare 
	sum_payment_amount();
	populate_unspent_list();
	// construct tx
	populate_tx_header();
	populate_tx_inputs();
	populate_tx_outputs();
	// check tx
	check_tx();
	// sign tx
	sign_tx_inputs();
	// send tx
	send_tx();
}
tx_type& base_transfer_helper::get_transaction(){
	return tx_;
}

// copy from src/lib/consensus/clone/script/script.h
std::vector<unsigned char> base_transfer_helper::satoshi_to_chunk(const int64_t& value)
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
		
//const std::vector<uint16_t> libbitcoin::explorer::commands::depositing_etp::vec_cycle{7, 30, 90, 182, 365};
const std::vector<uint16_t> depositing_etp::vec_cycle{7, 30, 90, 182, 365};

void depositing_etp::populate_change() {
	if(from_.empty())
		receiver_list_.push_back({from_list_.at(0).addr, "", unspent_etp_ - payment_etp_, 0, asset::asset_status::asset_none});
	else
		receiver_list_.push_back({from_, "", unspent_etp_ - payment_etp_, 0, asset::asset_status::asset_none});
}

uint32_t depositing_etp::get_reward_lock_height() {
	int index = 0;
	auto it = std::find(vec_cycle.begin(), vec_cycle.end(), deposit_cycle_);
	if (it == vec_cycle.end()) { // not found cycle
		index = 0;
	} else {
		index = std::distance(vec_cycle.begin(), it);
	}

	return (uint32_t)bc::consensus::lock_heights[index];
}
// modify lock script
void depositing_etp::populate_tx_outputs() {
	chain::operation::stack payment_ops;
	
    for (auto& iter: receiver_list_) {
        if (tx_item_idx_ >= (tx_limit + 10)) {
                throw std::runtime_error{"Too many inputs/outputs makes tx too large, canceled."};
        }
		tx_item_idx_++;
		
		// filter zero etp and asset
		if( !iter.amount && !iter.asset_amount)
			continue;
		
		// complicated script and asset should be implemented in subclass
		// generate script			
		const wallet::payment_address payment(iter.target);
		if (!payment)
			throw std::logic_error{"invalid target address"};
		auto hash = payment.hash();
		if((to_ == iter.target)
			&& (asset::asset_status::asset_locked == iter.status)) {// borrow asset status field in pure etp tx
			payment_ops = chain::operation::to_pay_key_hash_with_lock_height_pattern(hash, get_reward_lock_height());
		} else {
			payment_ops = chain::operation::to_pay_key_hash_pattern(hash); // common payment script
		}
			
		auto payment_script = chain::script{ payment_ops };
		
		// generate asset info
		auto output_att = populate_output_attachment(iter);
		
		// fill output
		tx_.outputs.push_back({ iter.amount, payment_script, output_att });
    }
}

		
void sending_etp::populate_change() {
	if(from_.empty())
		receiver_list_.push_back({from_list_.at(0).addr, "", unspent_etp_ - payment_etp_, 0, asset::asset_status::asset_none});
	else
		receiver_list_.push_back({from_, "", unspent_etp_ - payment_etp_, 0, asset::asset_status::asset_none});
}

void sending_multisig_etp::populate_change() {
	if(from_.empty())
		receiver_list_.push_back({from_list_.at(0).addr, "", unspent_etp_ - payment_etp_, 0, asset::asset_status::asset_none});
	else
		receiver_list_.push_back({from_, "", unspent_etp_ - payment_etp_, 0, asset::asset_status::asset_none});
}
#include <metaverse/bitcoin/config/base16.hpp>

void sending_multisig_etp::sign_tx_inputs() {
    uint32_t index = 0;
	std::vector<std::string> hd_pubkeys;
	std::string prikey, pubkey, multisig_script;
	
    for (auto& fromeach : from_list_){
		// populate unlock script
		hd_pubkeys.clear();
		for(auto& each : multisig_pubkeys_) {
			get_multisig_pri_pub_key(prikey, pubkey, each, fromeach.hd_index);
			hd_pubkeys.push_back(pubkey);
		}
		multisig_script = get_multisig_script(m_, n_, hd_pubkeys);
		log::trace("wdy script=") << multisig_script;
		wallet::payment_address payment("3JoocenkYHEKFunupQSgBUR5bDWioiTq5Z");
		log::trace("wdy hash=") << libbitcoin::config::base16(payment.hash());
		// prepare sign
		explorer::config::hashtype sign_type;
		uint8_t hash_type = (signature_hash_algorithm)sign_type;
		
		bc::explorer::config::ec_private config_private_key(fromeach.prikey);
		const ec_secret& private_key =	  config_private_key;	 
		
		bc::explorer::config::script config_contract(multisig_script);
		const bc::chain::script& contract = config_contract;
		
		// gen sign
		bc::endorsement endorse;
		if (!bc::chain::script::create_endorsement(endorse, private_key,
			contract, tx_, index, hash_type))
		{
			throw std::logic_error{"get_input_sign sign failure"};
		}
		// do script
		bc::chain::script ss;
		data_chunk data;
		ss.operations.push_back({bc::chain::opcode::zero, data});
		ss.operations.push_back({bc::chain::opcode::special, endorse});
		//ss.operations.push_back({bc::chain::opcode::special, endorse2});

		chain::script script_encoded;
		script_encoded.from_string(multisig_script);
		
		ss.operations.push_back({bc::chain::opcode::pushdata1, script_encoded.to_data(false)});
		
        // set input script of this tx
        tx_.inputs[index].script = ss;
        index++;
    }

}

void sending_multisig_etp::update_tx_inputs_signature() {
	#if 0
    uint32_t index = 0;
	std::vector<std::string> hd_pubkeys;
	std::string prikey, pubkey, multisig_script;
	
    for (auto& fromeach : from_list_){
		// populate unlock script
		hd_pubkeys.clear();
		for(auto& each : multisig_pubkeys_) {
			get_multisig_pri_pub_key(prikey, pubkey, each, fromeach.hd_index);
			hd_pubkeys.push_back(pubkey);
		}
		multisig_script = get_multisig_script(m_, n_, hd_pubkeys);
		log::trace("wdy script=") << multisig_script;
		wallet::payment_address payment("3JoocenkYHEKFunupQSgBUR5bDWioiTq5Z");
		log::trace("wdy hash=") << libbitcoin::config::base16(payment.hash());
		// prepare sign
		explorer::config::hashtype sign_type;
		uint8_t hash_type = (signature_hash_algorithm)sign_type;
		
		bc::explorer::config::ec_private config_private_key(fromeach.prikey);
		const ec_secret& private_key =	  config_private_key;	 
		
		bc::explorer::config::script config_contract(multisig_script);
		const bc::chain::script& contract = config_contract;
		
		// gen sign
		bc::endorsement endorse;
		if (!bc::chain::script::create_endorsement(endorse, private_key,
			contract, tx_, index, hash_type))
		{
			throw std::logic_error{"get_input_sign sign failure"};
		}
		// do script
		bc::chain::script ss;
		data_chunk data;
		ss.operations.push_back({bc::chain::opcode::zero, data});
		ss.operations.push_back({bc::chain::opcode::special, endorse});
		//ss.operations.push_back({bc::chain::opcode::special, endorse2});

		chain::script script_encoded;
		script_encoded.from_string(multisig_script);
		
		ss.operations.push_back({bc::chain::opcode::pushdata1, script_encoded.to_data(false)});
		
        // set input script of this tx
        tx_.inputs[index].script = ss;
        index++;
    }
	#endif
	// get all address of this account
	auto pvaddr = blockchain_.get_account_addresses(name_);
	if(!pvaddr) 
		throw std::logic_error{"nullptr for address list"};
	bc::chain::script ss;
	bc::chain::script redeem_script;
    uint32_t hd_index;

	std::vector<std::string> hd_pubkeys;
	std::string prikey, pubkey, multisig_script, addr_prikey;
    uint32_t index = 0;
	for(auto& each_input : tx_.inputs) {
		ss = each_input.script;
		log::trace("wdy old script=") << ss.to_string(false);
		const auto& ops = ss.operations;
		
		// 1. extract address from multisig payment script
		// zero sig1 sig2 ... encoded-multisig
		const auto& redeem_data = ops.back().data;
		
		if (redeem_data.empty())
			throw std::logic_error{"empty redeem script."};
		
		if (!redeem_script.from_data(redeem_data, false, bc::chain::script::parse_mode::strict))
			throw std::logic_error{"error occured when parse redeem script data."};
		
		// Is the redeem script a standard pay (output) script?
		const auto redeem_script_pattern = redeem_script.pattern();
		if(redeem_script_pattern != script_pattern::pay_multisig)
			throw std::logic_error{"redeem script is not pay multisig pattern."};
		
		const payment_address address(redeem_script, 5);
		auto addr_str = address.encoded(); // pay address
		
		// 2. get address prikey/hd_index
		#if 0
		auto it = std::find(pvaddr->begin(), pvaddr->end(), addr_str);
		if(it == pvaddr->end())
			throw std::logic_error{std::string("not found address : ") + addr_str};
		addr_prikey = it->get_prv_key(passwd_);
		hd_index = it->get_hd_index();
		#endif
		addr_prikey = "";
		hd_index = 0xffffffff;
		for (auto& each : *pvaddr){
			if ( addr_str == each.get_address() ) { // find address
				addr_prikey = each.get_prv_key(passwd_);
				hd_index = each.get_hd_index();
				break;
			}
		}
		if((hd_index == 0xffffffff) && addr_prikey.empty())
			throw std::logic_error{std::string("not found address : ") + addr_str};
		// 3. populate unlock script
		hd_pubkeys.clear();
		for(auto& each : multisig_pubkeys_) {
			get_multisig_pri_pub_key(prikey, pubkey, each, hd_index);
			hd_pubkeys.push_back(pubkey);
		}
		multisig_script = get_multisig_script(m_, n_, hd_pubkeys);
		log::trace("wdy script=") << multisig_script;
		//wallet::payment_address payment("3JoocenkYHEKFunupQSgBUR5bDWioiTq5Z");
		//log::trace("wdy hash=") << libbitcoin::config::base16(payment.hash());
		// prepare sign
		explorer::config::hashtype sign_type;
		uint8_t hash_type = (signature_hash_algorithm)sign_type;
		
		bc::explorer::config::ec_private config_private_key(addr_prikey);
		const ec_secret& private_key =	  config_private_key;	 
		
		bc::explorer::config::script config_contract(multisig_script);
		const bc::chain::script& contract = config_contract;
		
		// gen sign
		bc::endorsement endorse;
		if (!bc::chain::script::create_endorsement(endorse, private_key,
			contract, tx_, index, hash_type))
		{
			throw std::logic_error{"get_input_sign sign failure"};
		}
		// insert endorse
		auto position = ss.operations.begin();
		ss.operations.insert(position + 1, {bc::chain::opcode::special, endorse});
		
        // set input script of this tx
        each_input.script = ss;
		log::trace("wdy new script=") << ss.to_string(false);
	}

}
void issuing_asset::sum_payment_amount() {
	if(receiver_list_.empty())
		throw std::logic_error{"empty target address"};
	//if (payment_etp_ < maximum_fee)
	if (payment_etp_ < 1000000000) // test code 10 etp now
		throw std::logic_error{"fee must more than 10000000000 satoshi == 100 etp"};

	for (auto& iter : receiver_list_) {
		payment_etp_ += iter.amount;
		payment_asset_ += iter.asset_amount;
	}
}

void issuing_asset::populate_change() {
	if(from_.empty())
		receiver_list_.push_back({from_list_.at(0).addr, "", unspent_etp_ - payment_etp_, 0, asset::asset_status::asset_none});
	else
		receiver_list_.push_back({from_, "", unspent_etp_ - payment_etp_, 0, asset::asset_status::asset_none});
}
void issuing_locked_asset::sum_payment_amount() {
	if(receiver_list_.empty())
		throw std::logic_error{"empty target address"};
	//if (payment_etp_ < maximum_fee)
	if (payment_etp_ < 1000000000) // test code 10 etp now
		throw std::logic_error{"fee must more than 10000000000 satoshi == 100 etp"};

	for (auto& iter : receiver_list_) {
		payment_etp_ += iter.amount;
		payment_asset_ += iter.asset_amount;
	}
}

void issuing_locked_asset::populate_change() {
	if(from_.empty())
		receiver_list_.push_back({from_list_.at(0).addr, "", unspent_etp_ - payment_etp_, 0, asset::asset_status::asset_none});
	else
		receiver_list_.push_back({from_, "", unspent_etp_ - payment_etp_, 0, asset::asset_status::asset_none});
}
uint32_t issuing_locked_asset::get_lock_height(){
	uint64_t height = (deposit_cycle_*24)*(3600/24);
	if(0xffffffff <= height)
		throw std::logic_error{"lock time outofbound!"};
	return static_cast<uint32_t>(height);
}
// modify lock script
void issuing_locked_asset::populate_tx_outputs() {
	chain::operation::stack payment_ops;
	
	for (auto iter = receiver_list_.begin(); iter != receiver_list_.end(); iter++) {
        if (tx_item_idx_ >= (tx_limit + 10)) {
                throw std::runtime_error{"Too many inputs/outputs makes tx too large, canceled."};
        }
		tx_item_idx_++;
		
		// filter zero etp and asset. status check just for issue asset
		if( !iter->amount && !iter->asset_amount && (iter->status != asset::asset_status::asset_locked))
			continue;
		
		// complicated script and asset should be implemented in subclass
		// generate script			
		const wallet::payment_address payment(iter->target);
		if (!payment)
			throw std::logic_error{"invalid target address"};
		auto hash = payment.hash();
		if(iter == (receiver_list_.end() - 1)) { // change record
			payment_ops = chain::operation::to_pay_key_hash_pattern(hash); // common payment script
		} else {
			if(deposit_cycle_)
				payment_ops = chain::operation::to_pay_key_hash_with_lock_height_pattern(hash, get_lock_height());
			else
				payment_ops = chain::operation::to_pay_key_hash_pattern(hash); // common payment script
		}
			
		auto payment_script = chain::script{ payment_ops };
		
		// generate asset info
		auto output_att = populate_output_attachment(*iter);
		
		// fill output
		tx_.outputs.push_back({ iter->amount, payment_script, output_att });
    }
}
void sending_asset::populate_change() {
	if(from_.empty())
		receiver_list_.push_back({from_list_.at(0).addr, symbol_, unspent_etp_ - payment_etp_, unspent_asset_ - payment_asset_,
		asset::asset_status::asset_transferable});
	else
		receiver_list_.push_back({from_, symbol_, unspent_etp_ - payment_etp_, unspent_asset_ - payment_asset_,
		asset::asset_status::asset_transferable});
}
void sending_locked_asset::populate_change() {
	if(from_.empty())
		receiver_list_.push_back({from_list_.at(0).addr, symbol_, unspent_etp_ - payment_etp_, unspent_asset_ - payment_asset_,
		asset::asset_status::asset_transferable});
	else
		receiver_list_.push_back({from_, symbol_, unspent_etp_ - payment_etp_, unspent_asset_ - payment_asset_,
		asset::asset_status::asset_transferable});
}
uint32_t sending_locked_asset::get_lock_height(){
	uint64_t height = (deposit_cycle_*24)*(3600/24);
	if(0xffffffff <= height)
		throw std::logic_error{"lock time outofbound!"};
	return static_cast<uint32_t>(height);
}
// modify lock script
void sending_locked_asset::populate_tx_outputs() {
	chain::operation::stack payment_ops;
	
    for (auto iter = receiver_list_.begin(); iter != receiver_list_.end(); iter++) {
        if (tx_item_idx_ >= (tx_limit + 10)) {
                throw std::runtime_error{"Too many inputs/outputs makes tx too large, canceled."};
        }
		tx_item_idx_++;
		
		// filter zero etp and asset. status check just for issue asset
		if( !iter->amount && !iter->asset_amount && (iter->status != asset::asset_status::asset_locked))
			continue;
		
		// complicated script and asset should be implemented in subclass
		// generate script			
		const wallet::payment_address payment(iter->target);
		if (!payment)
			throw std::logic_error{"invalid target address"};
		auto hash = payment.hash();
		if(iter == (receiver_list_.end() - 1)) { // change record
			payment_ops = chain::operation::to_pay_key_hash_pattern(hash); // common payment script
		} else {
			if(deposit_cycle_)
				payment_ops = chain::operation::to_pay_key_hash_with_lock_height_pattern(hash, get_lock_height());
			else
				payment_ops = chain::operation::to_pay_key_hash_pattern(hash); // common payment script
		}
			
		auto payment_script = chain::script{ payment_ops };
		
		// generate asset info
		auto output_att = populate_output_attachment(*iter);
		
		// fill output
		tx_.outputs.push_back({ iter->amount, payment_script, output_att });
    }
}

}// commands
}// explorer
}// libbitcoin
