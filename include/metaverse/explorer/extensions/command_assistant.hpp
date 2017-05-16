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

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <array>

#include <metaverse/bitcoin.hpp>

#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/command.hpp>
#include <metaverse/explorer/extensions/command_extension.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>


namespace libbitcoin {
namespace explorer {
namespace commands{

struct address_asset_record{
	std::string prikey;
	std::string addr;
	uint64_t	amount; // spendable etp amount
	std::string symbol;
	uint64_t	asset_amount; // spendable asset amount
	asset::asset_status status; // only used for non-etp asset
	output_point output;
	chain::script script;
	uint32_t hd_index; // only used for multisig tx
};

struct balances {
	uint64_t total_received;
	uint64_t confirmed_balance;
	uint64_t unspent_balance;
	uint64_t frozen_balance;
};

typedef std::pair<std::string, uint64_t> address_amount;
class BCX_API utxo_helper;
class BCX_API utxo_attach_issue_helper;
class BCX_API utxo_attach_send_helper;
class BCX_API utxo_attach_issuefrom_helper;
class BCX_API utxo_attach_sendfrom_helper;
// to genarate address
std::string ec_to_xxx_impl(const char* commands, const std::string& fromkey, bool use_testnet_rules = false);

// public send
void get_tx_decode(const std::string& tx_set, std::string& tx_decode);
void validate_tx(const std::string& tx_set);
void send_tx(const std::string& tx_set, std::string& send_ret);
uint64_t get_total_payment_amount(const std::vector<std::string>& receiver_list, address_amount& mychange);
std::string get_multisig_script(uint8_t m, uint8_t n, std::vector<std::string>& public_keys);
void get_multisig_pri_pub_key(std::string& prikey, std::string& pubkey, std::string& seed, uint32_t hd_index);
history::list expand_history(history_compact::list& compact);
history::list get_address_history(wallet::payment_address& addr, bc::blockchain::block_chain_impl& blockchain);
void expand_history(history_compact::list& compact, history::list& result);
void get_address_history(wallet::payment_address& addr, bc::blockchain::block_chain_impl& blockchain,
	history::list& history_vec);
chain::points_info sync_fetchutxo(uint64_t amount, wallet::payment_address& addr, 
	std::string& type, bc::blockchain::block_chain_impl& blockchain);
void sync_fetchbalance (wallet::payment_address& address, 
	std::string& type, bc::blockchain::block_chain_impl& blockchain, balances& addr_balance, uint64_t amount);

// etp
bool send_impl(utxo_helper& utxo, bc::blockchain::block_chain_impl& blockchain, std::ostream& output, std::ostream& cerr);
// asset
bool send_impl(utxo_attach_issue_helper& utxo, bc::blockchain::block_chain_impl& blockchain, 
        std::ostream& output, std::ostream& cerr);
bool send_impl(utxo_attach_send_helper& utxo, bc::blockchain::block_chain_impl& blockchain, 
        std::ostream& output, std::ostream& cerr);
bool send_impl(utxo_attach_issuefrom_helper& utxo, bc::blockchain::block_chain_impl& blockchain, 
    std::ostream& output, std::ostream& cerr);
bool send_impl(utxo_attach_sendfrom_helper& utxo, bc::blockchain::block_chain_impl& blockchain, 
    std::ostream& output, std::ostream& cerr);

// ---------------------------------------------------------------------------

// previous tx outputs
struct tx_outputs_fields{

    std::string index;
    std::string script;
    uint64_t value{0};

    std::string as_tx_encode_input_args;
    std::string as_input_sign;
    uint8_t     script_version{0};
};

// previous tx
struct tx_items{

    std::string txhash{""};
    std::string lock_time{""};
    uint32_t    version{0};
    tx_outputs_fields output;
};

struct  addr_etp_amount {
    std::string addr; 
    uint64_t etp_value;
    uint64_t asset_amount;
};

// ---------------------------------------------------------------------------

class BCX_API utxo_helper 
{

public:
    explicit utxo_helper(const prikey_amount& from, const std::string& receiver)
        { 
            keys_inputs_.emplace(std::make_pair(from.first, std::vector<tx_items>()));
            receiver_list_.push_back(receiver);
        }
    explicit utxo_helper(std::list<prikey_amount>&& from_list, std::vector<std::string>&& receiver_list):
        from_list_(from_list),
        receiver_list_(receiver_list)
        {
            for (auto& each : from_list_){
                keys_inputs_.emplace(std::make_pair(each.first, std::vector<tx_items>()));
            }
        }

    void get_payment_by_receivers(){
        total_payment_amount_ = get_total_payment_amount(receiver_list_, mychange_);
    }
    uint64_t get_my_balance();
    void group_utxo();
    inline void set_mychange_by_threshold(std::string& mychange);

    bool fetch_utxo(std::string& change, bc::blockchain::block_chain_impl& blockchain);
    bool fetch_tx();
    void get_tx_encode(std::string& tx_encode);
    void get_input_sign(std::string& tx_encode);
    void get_input_set(const std::string& tx_encode, std::string& tx_set);
    void set_testnet_rules(bool outside){ is_testnet_rules = outside; }
    void set_reward(uint16_t reward_in){ reward_in_ = reward_in; }
	uint32_t get_reward_lock_block_height();

    static const uint64_t maximum_fee{10000000000};
    static const uint64_t minimum_fee{10000};

private:
    bool                        is_testnet_rules{false};
    uint16_t                    reward_in_{0};
	std::string                 period_str;
    uint64_t                    total_payment_amount_{0};
    address_amount              mychange_;

    // to
    std::vector<std::string>    receiver_list_;

    // from
    std::list<prikey_amount>    from_list_;// put xxx.first as keys_inputs_ key
    std::unordered_map<std::string, std::vector<tx_items>> keys_inputs_;
};

class BCX_API utxo_attach_issue_helper 
{

public:
    explicit utxo_attach_issue_helper(std::string&& name, std::string&& passwd, std::string&& type,
        std::list<prikey_amount>&& from_list, std::vector<utxo_attach_info>&& receiver_list, uint64_t fee,
        std::string&& symbol, uint64_t amount):
        name_(name), passwd_(passwd), business_type_(type), from_list_(from_list), receiver_list_(receiver_list), fee_(fee),
        symbol_(symbol), amount_(amount)
    {
        for (auto& each : from_list_){
            keys_inputs_.emplace(std::make_pair(each.first, std::vector<tx_items>()));
        }
    }
    void get_payment_by_receivers() {        
        for (auto& iter : receiver_list_)
        {
            total_payment_amount_ += iter.value;
        }
        total_payment_amount_ += fee_;
    }
    uint64_t get_my_balance();
    bool group_utxo();
    //inline void set_mychange_by_threshold(std::string& tgt_addr, uint64_t threshold);
    bool fetch_utxo(std::string& change, bc::blockchain::block_chain_impl& blockchain);
    bool fetch_tx();
    void get_tx_encode(std::string& tx_encode, bc::blockchain::block_chain_impl& blockchain);
    void get_input_sign(std::string& tx_encode);
    void get_input_set(const std::string& tx_encode, std::string& tx_set);
    void get_utxo_option(utxo_attach_info& info);
private:
    std::string                 name_;
    std::string                 passwd_;
    std::string                 business_type_;
    uint64_t                     fee_{0};
    std::string                    symbol_;
    uint64_t                     amount_;
    uint64_t                    total_payment_amount_{0};
    address_amount              mychange_;

    // to
    std::vector<utxo_attach_info>    receiver_list_;

    // from
    std::list<prikey_amount>    from_list_;// put xxx.first as keys_inputs_ key
    std::unordered_map<std::string, std::vector<tx_items>> keys_inputs_;
};

class BCX_API utxo_attach_send_helper 
{
public:
    explicit utxo_attach_send_helper(std::string&& name, std::string&& passwd, std::string&& type,
        std::list<prikey_amount>&& etp_ls, std::list<prikey_etp_amount>&& asset_ls, uint64_t fee,
        std::string&& symbol, uint64_t amount, std::string&& address):
        name_(name), passwd_(passwd), business_type_(type), etp_ls_(etp_ls), asset_ls_(asset_ls), total_payment_amount_(fee),
        symbol_(symbol), amount_(amount), address_(address)
    {
        for (auto& each : etp_ls_){
            if(keys_inputs_.find(each.first) == keys_inputs_.end())
                keys_inputs_.emplace(std::make_pair(each.first, std::vector<tx_items>()));
        }
        for (auto& each : asset_ls_){
            if(keys_inputs_.find(each.key) == keys_inputs_.end())
                keys_inputs_.emplace(std::make_pair(each.key, std::vector<tx_items>()));
        }
    }
    
    inline void set_mychange(std::string& addr, uint64_t change) {
        mychange_.first = addr;
        mychange_.second = change;
    }
    uint64_t get_my_balance();
    bool group_utxo();
    //inline void set_mychange_by_threshold(std::string& tgt_addr, uint64_t threshold);
    bool fetch_utxo(bc::blockchain::block_chain_impl& blockchain);
    bool fetch_tx();
    void get_tx_encode(std::string& tx_encode, bc::blockchain::block_chain_impl& blockchain);
    void get_input_sign(std::string& tx_encode);
    void get_input_set(const std::string& tx_encode, std::string& tx_set);
    void get_utxo_option(utxo_attach_info& info);
    bool fetch_utxo_impl(bc::blockchain::block_chain_impl& blockchain,
		std::string& prv_key, uint64_t payment_amount, uint64_t& utxo_change);
    void generate_receiver_list();
    uint64_t get_etp_balance();
    uint64_t get_asset_balance();
    uint64_t get_asset_amounts();
    void group_asset_amount();
    void group_asset_etp_amount(uint64_t etp_amount);
    void group_etp();
    bool is_cmd_exist(const char* cmds[], size_t len, char* cmd);
    
private:
    std::string                 name_;
    std::string                 passwd_;
    std::string                 business_type_;
    //uint64_t                     fee_{0};
    std::string                    symbol_;
    uint64_t                     amount_;
    std::string                    address_;
    prikey_amount               address_pa_;
    uint64_t                    total_payment_amount_{0};
    address_amount              mychange_;
    //business_kind                kind_;
    //uint64_t                    total_amount_;
    // business
    std::list<prikey_amount> etp_ls_;
    std::list<prikey_etp_amount> asset_ls_;
    // to
    std::vector<utxo_attach_info>    receiver_list_;

    // from
    //std::list<prikey_amount>    from_list_;// put xxx.first as keys_inputs_ key
    std::unordered_map<std::string, std::vector<tx_items>> keys_inputs_;
};


class BCX_API utxo_attach_issuefrom_helper 
{

public:
    explicit utxo_attach_issuefrom_helper(std::string&& name, std::string&& passwd, std::string&& type,
        std::list<prikey_amount>&& from_list, std::string&& receiver, uint64_t fee,
        std::string&& symbol, uint64_t amount, bool use_testnet):
        name_(name), passwd_(passwd), business_type_(type), from_list_(from_list), receiver_(receiver), 
        total_payment_amount_(fee),    symbol_(symbol), amount_(amount), use_testnet_(use_testnet)
    {
        for (auto& each : from_list_){
            keys_inputs_.emplace(std::make_pair(each.first, std::vector<tx_items>()));
        }
    }
    uint64_t get_my_balance();
    void group_utxo();
    bool fetch_utxo(bc::blockchain::block_chain_impl& blockchain);
    bool fetch_tx();
    void get_tx_encode(std::string& tx_encode, bc::blockchain::block_chain_impl& blockchain);
    void get_input_sign(std::string& tx_encode);
    void get_input_set(const std::string& tx_encode, std::string& tx_set);
    void get_utxo_option(utxo_attach_info& info);
    inline void set_mychange(std::string& addr, uint64_t change) {
        mychange_.first = addr;
        mychange_.second = change;
    }
    void generate_receiver_list();

private:
    std::string                 name_;
    std::string                 passwd_;
    std::string                 business_type_;
    std::string                    symbol_;
    uint64_t                     amount_;
    uint64_t                    total_payment_amount_{0};
    address_amount              mychange_;
    bool                        use_testnet_;

    // to
    std::string                    receiver_;
    std::vector<utxo_attach_info>    receiver_list_;

    // from
    std::list<prikey_amount>    from_list_;// put xxx.first as keys_inputs_ key
    std::unordered_map<std::string, std::vector<tx_items>> keys_inputs_;
};


class BCX_API utxo_attach_sendfrom_helper 
{
public:
    explicit utxo_attach_sendfrom_helper(std::string&& name, std::string&& passwd, std::string&& type,
        std::list<prikey_etp_amount>&& asset_ls, uint64_t fee,
        std::string&& symbol, uint64_t amount, std::string&& address, bool use_testnet):
        name_(name), passwd_(passwd), business_type_(type), asset_ls_(asset_ls), total_payment_amount_(fee),
        symbol_(symbol), amount_(amount), address_(address), use_testnet_(use_testnet)
    {
        for (auto& each : asset_ls_){
			prikey_set_.insert(each.key);
            if(keys_inputs_.find(each.key) == keys_inputs_.end())
                keys_inputs_.emplace(std::make_pair(each.key, std::vector<tx_items>()));
        }
    }
    
    inline void set_mychange(std::string& addr, uint64_t change, uint64_t asset_amount) {
        mychange_.addr = addr;
        mychange_.etp_value = change;
        mychange_.asset_amount = asset_amount;
    }
    uint64_t get_my_balance();
    void group_utxo();
    //inline void set_mychange_by_threshold(std::string& tgt_addr, uint64_t threshold);
    bool fetch_utxo();
    bool fetch_tx();
    void get_tx_encode(std::string& tx_encode, bc::blockchain::block_chain_impl& blockchain);
    void get_input_sign(std::string& tx_encode);
    void get_input_set(const std::string& tx_encode, std::string& tx_set);
    void get_utxo_option(utxo_attach_info& info);
    bool fetch_utxo_impl(bc::blockchain::block_chain_impl& blockchain,
		std::string& prv_key, uint64_t payment_amount, uint64_t& utxo_change);
    void generate_receiver_list();
    uint64_t get_asset_balance();
    uint64_t get_asset_amounts();
    //bool is_cmd_exist(const char* cmds[], size_t len, char* cmd);
    
private:
    std::string                 name_;
    std::string                 passwd_;
    std::string                 business_type_;
    std::string                    symbol_;
    uint64_t                     amount_;
    std::string                    address_; // to address
    prikey_amount               address_pa_;
    uint64_t                    total_payment_amount_{0};
    //address_amount              mychange_;
    addr_etp_amount              mychange_;
    bool                         use_testnet_;
    //business_kind                kind_;
    //uint64_t                    total_amount_;
    // business
    std::list<prikey_etp_amount> asset_ls_;
	std::unordered_set<std::string> prikey_set_; // incase asset_ls_ has multil duplicated key
    // to
    std::vector<utxo_attach_info>    receiver_list_;

    // from
    //std::list<prikey_amount>    from_list_;// put xxx.first as keys_inputs_ key
    std::unordered_map<std::string, std::vector<tx_items>> keys_inputs_;
};
class BCX_API base_transfer_helper 
{
public:
	base_transfer_helper(bc::blockchain::block_chain_impl& blockchain, std::string&& name, std::string&& passwd, 
		std::string&& from, std::vector<receiver_record>&& receiver_list, uint64_t fee, std::string&& symbol = std::string("")):
		blockchain_{blockchain},
		name_{name},
		passwd_{passwd},
		from_{from},
		receiver_list_{receiver_list},
		payment_etp_{fee},
		symbol_{symbol}
		{
		};

	virtual ~base_transfer_helper(){
		receiver_list_.clear();
		from_list_.clear();
	};

	static const uint64_t maximum_fee{10000000000};
	static const uint64_t minimum_fee{10000};
	static const uint64_t tx_limit{677};
	static const uint64_t attach_version{1};
	
	virtual void sum_payment_amount();
	virtual void sync_fetchutxo (const std::string& prikey, const std::string& addr, uint32_t hd_index);
	virtual void populate_unspent_list();
	virtual void populate_change() = 0; 

	virtual void populate_tx_header(){
	    tx_.version = 1;
	    tx_.locktime = 0;
	};

	virtual void populate_tx_inputs();
	virtual attachment populate_output_attachment(receiver_record& record);
	virtual void populate_tx_outputs();
	virtual void check_tx();
	virtual void sign_tx_inputs();
	void send_tx();
	void exec();
	tx_type& get_transaction();
	std::vector<unsigned char> satoshi_to_chunk(const int64_t& value);
			
protected:
	tx_type                           tx_; // target transaction
	bc::blockchain::block_chain_impl& blockchain_;
	std::string                       name_;
	std::string                       passwd_;
	std::string 				      symbol_;
	std::string                       from_;
	uint64_t					      payment_etp_{0};
	uint64_t					      payment_asset_{0};
	uint64_t                          unspent_etp_{0};
	uint64_t                          unspent_asset_{0};
	uint64_t                          tx_item_idx_{0};
    // to
    std::vector<receiver_record>      receiver_list_;
    // from
    std::vector<address_asset_record>   from_list_;// put xxx.first as keys_inputs_ key
};

class BCX_API depositing_etp : public base_transfer_helper
{
public:
	depositing_etp(bc::blockchain::block_chain_impl& blockchain, std::string&& name, std::string&& passwd, 
		std::string&& to, std::vector<receiver_record>&& receiver_list, uint16_t deposit_cycle = 7, uint64_t fee = 10000):
		base_transfer_helper(blockchain, std::move(name), std::move(passwd), std::string(""), std::move(receiver_list), fee), 
		to_{to}, deposit_cycle_{deposit_cycle}
		{};

	~depositing_etp(){};
		
	static const std::vector<uint16_t> vec_cycle;
		
	void populate_change() override;
	
	uint32_t get_reward_lock_height();
	// modify lock script
	void populate_tx_outputs() override ;

private:
	std::string                       to_;
	uint16_t					      deposit_cycle_{7}; // 7 days
};

class BCX_API sending_etp : public base_transfer_helper
{
public:
	sending_etp(bc::blockchain::block_chain_impl& blockchain, std::string&& name, std::string&& passwd, 
		std::string&& from, std::vector<receiver_record>&& receiver_list, uint64_t fee):
		base_transfer_helper(blockchain, std::move(name), std::move(passwd), std::move(from), std::move(receiver_list), fee)
		{};

	~sending_etp(){};
			
	void populate_change() override ;
};

class BCX_API sending_multisig_etp : public base_transfer_helper
{
public:
	sending_multisig_etp(bc::blockchain::block_chain_impl& blockchain, std::string&& name, std::string&& passwd, 
		std::string&& from, std::vector<receiver_record>&& receiver_list, uint64_t fee, 
		uint8_t m, uint8_t n, std::vector<std::string>&& multisig_pubkeys):
		base_transfer_helper(blockchain, std::move(name), std::move(passwd), std::move(from), std::move(receiver_list), fee),
		m_{m}, n_{n}, multisig_pubkeys_{multisig_pubkeys}
		{};

	~sending_multisig_etp(){};
			
	void populate_change() override ;
	void sign_tx_inputs() override ;
	void update_tx_inputs_signature() ;
private:
	uint8_t m_;
	uint8_t n_;
	std::vector<std::string> multisig_pubkeys_;
};

class BCX_API issuing_asset : public base_transfer_helper
{
public:
	issuing_asset(bc::blockchain::block_chain_impl& blockchain, std::string&& name, std::string&& passwd, 
		std::string&& from, std::string&& symbol, std::vector<receiver_record>&& receiver_list, uint64_t fee):
		base_transfer_helper(blockchain, std::move(name), std::move(passwd), std::move(from), std::move(receiver_list), 
			fee, std::move(symbol))
		{};

	~issuing_asset(){};
		
	void sum_payment_amount() override;
	
	void populate_change() override;
};
class BCX_API issuing_locked_asset : public base_transfer_helper
{
public:
	issuing_locked_asset(bc::blockchain::block_chain_impl& blockchain, std::string&& name, std::string&& passwd, 
		std::string&& from, std::string&& symbol, std::vector<receiver_record>&& receiver_list, uint64_t fee, 
		uint32_t deposit_cycle = 0):
		base_transfer_helper(blockchain, std::move(name), std::move(passwd), std::move(from), std::move(receiver_list), 
			fee, std::move(symbol)), deposit_cycle_{deposit_cycle}
		{};

	~issuing_locked_asset(){};
		
	void sum_payment_amount() override;
	
	void populate_change() override;
	// modify lock script
	void populate_tx_outputs() override ;
	uint32_t get_lock_height();
	
private:
	uint32_t					      deposit_cycle_{0};
};
class BCX_API sending_asset : public base_transfer_helper
{
public:
	sending_asset(bc::blockchain::block_chain_impl& blockchain, std::string&& name, std::string&& passwd, 
		std::string&& from, std::string&& symbol, std::vector<receiver_record>&& receiver_list, uint64_t fee):
		base_transfer_helper(blockchain, std::move(name), std::move(passwd), std::move(from), std::move(receiver_list), 
			fee, std::move(symbol))
		{};

	~sending_asset(){};
			
	void populate_change() override;
};
class BCX_API sending_locked_asset : public base_transfer_helper
{
public:
	sending_locked_asset(bc::blockchain::block_chain_impl& blockchain, std::string&& name, std::string&& passwd, 
		std::string&& from, std::string&& symbol, std::vector<receiver_record>&& receiver_list, uint64_t fee,
		uint32_t deposit_cycle = 0):
		base_transfer_helper(blockchain, std::move(name), std::move(passwd), std::move(from), std::move(receiver_list), 
			fee, std::move(symbol)), deposit_cycle_{deposit_cycle}
		{};

	~sending_locked_asset(){};
			
	void populate_change() override;
	// modify lock script
	void populate_tx_outputs() override ;
	uint32_t get_lock_height();

private:
	uint32_t						  deposit_cycle_{0};
};


} // commands
} // explorer
} // libbitcoin

