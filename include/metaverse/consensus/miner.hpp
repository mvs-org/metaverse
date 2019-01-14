/**
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse-consensus.
 *
 * metaverse-consensus is free software: you can redistribute it and/or
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

#ifndef MVS_CONSENSUS_MINER_HPP
#define MVS_CONSENSUS_MINER_HPP

#include <array>
#include <boost/thread.hpp>
#include <metaverse/bitcoin.hpp>
#include "metaverse/blockchain/transaction_pool.hpp"
#include "metaverse/bitcoin/chain/block.hpp"
#include "metaverse/bitcoin/chain/input.hpp"
#include <metaverse/bitcoin/chain/attachment/asset/blockchain_asset.hpp>
#include <metaverse/bitcoin/wallet/ec_public.hpp>
#include <metaverse/blockchain/settings.hpp>

namespace libbitcoin {
namespace node {
class p2p_node;
}
namespace blockchain {
class block_chain_impl;
}
}

namespace libbitcoin {
namespace consensus {

BC_CONSTEXPR uint32_t min_tx_fee_per_kb = 1000;

extern std::array<uint64_t, 5> lock_heights;
extern const std::array<uint16_t,5 > lock_cycles;

class miner
{
public:
    typedef message::block_message block;
    typedef std::shared_ptr<message::block_message> block_ptr;
    typedef chain::header header;
    typedef chain::transaction transaction;
    typedef message::transaction_message::ptr transaction_ptr;
    typedef blockchain::block_chain_impl block_chain_impl;
    typedef blockchain::transaction_pool transaction_pool;
    typedef libbitcoin::node::p2p_node p2p_node;

    // prev_output_point -> (prev_block_height, prev_output)
    typedef std::unordered_map<chain::point, std::pair<uint64_t, chain::output>> previous_out_map_t;

    // tx_hash -> tx_fee
    typedef std::unordered_map<hash_digest, uint64_t> tx_fee_map_t;

    miner(p2p_node& node);
    ~miner();

    enum state
    {
        init_,
        exit_
    };

    void set_pos_params(bool isStaking, const std::string& account, const std::string& passwd);
    bool start(const wallet::payment_address& pay_address, uint16_t number = 0);
    bool stop();
    static block_ptr create_genesis_block(bool is_mainnet);
    bool script_hash_signature_operations_count(uint64_t &count, const chain::input::list& inputs,
        std::vector<transaction_ptr>& transactions);
    bool script_hash_signature_operations_count(uint64_t &count, const chain::input& input,
        std::vector<transaction_ptr>& transactions);
    transaction_ptr create_coinbase_tx(const wallet::payment_address& pay_address,
        uint64_t value, uint64_t block_height, int lock_height);
    transaction_ptr create_coinstake_tx(
        const ec_secret& private_key,
        const wallet::payment_address& pay_address,
        block_ptr pblock, const chain::output_info::list& stake_outputs);
    bool sign_coinstake_tx(
        const ec_secret& private_key,
        transaction_ptr coinstake);
    transaction_ptr create_pos_genesis_tx(uint64_t block_height, uint32_t block_time);

    block_ptr get_block(bool is_force_create_block = false);
    bool get_work(std::string& seed_hash, std::string& header_hash, std::string& boundary);
    bool put_result(const std::string& nonce, const std::string& mix_hash,
        const std::string& header_hash, const uint64_t &nounce_mask);
    const wallet::payment_address& get_miner_payment_address() const;
    bool set_miner_payment_address(const wallet::payment_address& address);
    void get_state(uint64_t &height,  uint64_t &rate, std::string& difficulty, bool& is_mining);
    bool get_block_header(chain::header& block_header, const std::string& para);

    static chain::operation::stack to_script_operation(
        const wallet::payment_address& pay_address, uint64_t lock_height=0);

    static uint64_t get_reward_lock_height(uint16_t lock_cycle);
    static int get_lock_heights_index(uint64_t height);
    static uint64_t calculate_block_subsidy(uint64_t height, bool is_testnet, uint32_t version);
    static uint64_t calculate_block_subsidy_pow(uint64_t height, bool is_testnet);
    static uint64_t calculate_block_subsidy_pos(uint64_t height, bool is_testnet);
    static uint64_t calculate_block_subsidy_dpos(uint64_t height, bool is_testnet);
    static uint64_t calculate_lockblock_reward(uint64_t lcok_heights, uint64_t num);

    static uint64_t calculate_mst_subsidy(
        const blockchain_asset& mining_asset, const asset_cert& mining_cert,
        uint64_t height, bool is_testnet, uint32_t version);
    static uint64_t calculate_mst_subsidy_pow(
        const blockchain_asset& mining_asset, const asset_cert& mining_cert,
        uint64_t height, bool is_testnet);
    static uint64_t calculate_mst_subsidy_pos(
        const blockchain_asset& mining_asset, const asset_cert& mining_cert,
        uint64_t height, bool is_testnet);
    static uint64_t calculate_mst_subsidy_dpos(
        const blockchain_asset& mining_asset, const asset_cert& mining_cert,
        uint64_t height, bool is_testnet);

    chain::block_version get_accept_block_version() const;
    void set_accept_block_version(chain::block_version v);

    bool is_witness() const;
    bool set_pub_and_pri_key(const std::string& pubkey, const std::string& prikey);

    std::string get_mining_asset_symbol() const;
    bool set_mining_asset_symbol(const std::string& symbol);

private:
    void work(const wallet::payment_address& pay_address);
    block_ptr create_new_block(const wallet::payment_address& pay_address);
    block_ptr create_new_block_pow(const wallet::payment_address& pay_address, const header& prev_header);
    block_ptr create_new_block_pos(const wallet::payment_address& pay_address, const header& prev_header);
    block_ptr create_new_block_dpos(const wallet::payment_address& pay_address, const header& prev_header);

    uint32_t get_adjust_time(uint64_t height) const;
    bool get_transaction(uint64_t last_height, std::vector<transaction_ptr>&, previous_out_map_t&, tx_fee_map_t&) const;
    bool get_block_transactions(
        uint64_t last_height, std::vector<transaction_ptr>& txs, std::vector<transaction_ptr>& reward_txs,
        uint64_t& total_fee, uint32_t& total_tx_sig_length);
    uint64_t store_block(block_ptr block);
    uint64_t get_height() const;
    bool get_input_etp(const transaction&, const std::vector<transaction_ptr>&, uint64_t&, previous_out_map_t&) const ;
    bool is_stop_miner(uint64_t block_height, block_ptr block) const;
    uint32_t get_tx_sign_length(transaction_ptr tx);
    void sleep_for_mseconds(uint32_t interval, bool force = false);

    u256 get_next_target_required(const chain::header& header, const chain::header& prev_header);

    std::shared_ptr<chain::output> create_coinbase_mst_output(
        const wallet::payment_address& pay_address, const std::string& symbol, uint64_t value);
    bool add_coinbase_mst_output(chain::transaction& coinbase_tx,
        const wallet::payment_address& pay_address, uint64_t block_height, uint32_t version);

private:
    p2p_node& node_;
    std::shared_ptr<boost::thread> thread_;
    mutable state state_;
    uint16_t new_block_number_;
    uint16_t new_block_limit_;
    chain::block_version accept_block_version_;
    std::shared_ptr<blockchain_asset> mining_asset_;
    std::shared_ptr<asset_cert> mining_cert_;

    block_ptr new_block_;
    wallet::payment_address pay_address_;
    const blockchain::settings& setting_;
    data_chunk public_key_data_;
    ec_secret private_key_;
};

}
}

#endif
