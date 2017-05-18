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
#pragma once

#include <metaverse/bitcoin.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/command.hpp>

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

struct receiver_record {
    std::string target;
	std::string symbol;
    uint64_t    amount; // etp value
    uint64_t    asset_amount;
	asset::asset_status status; // only used for non-etp asset
};

struct balances {
	uint64_t total_received;
	uint64_t confirmed_balance;
	uint64_t unspent_balance;
	uint64_t frozen_balance;
};
// helper function
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
