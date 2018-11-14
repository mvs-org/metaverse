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

#include <vector>
#include <boost/thread.hpp>

#include "metaverse/blockchain/transaction_pool.hpp"
#include "metaverse/bitcoin/chain/block.hpp"
#include "metaverse/bitcoin/chain/input.hpp"
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

BC_CONSTEXPR unsigned int min_tx_fee_per_kb = 1000;
BC_CONSTEXPR unsigned int median_time_span = 11;
BC_CONSTEXPR uint64_t future_blocktime_fork_height = 1030000;

extern int bucket_size;
extern std::vector<uint64_t> lock_heights;

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
    bool start(const std::string& pay_public_key, uint16_t number = 0);
    bool stop();
    static block_ptr create_genesis_block(bool is_mainnet);
    bool script_hash_signature_operations_count(size_t &count, const chain::input::list& inputs,
        std::vector<transaction_ptr>& transactions);
    bool script_hash_signature_operations_count(size_t &count, const chain::input& input,
        std::vector<transaction_ptr>& transactions);
    transaction_ptr create_coinbase_tx(const wallet::payment_address& pay_addres,
        uint64_t value, uint64_t block_height, int lock_height, uint32_t reward_lock_time);

    block_ptr get_block(bool is_force_create_block = false);
    bool get_work(std::string& seed_hash, std::string& header_hash, std::string& boundary);
    bool put_result(const std::string& nonce, const std::string& mix_hash,
        const std::string& header_hash, const uint64_t &nounce_mask);
    bool set_miner_payment_address(const wallet::payment_address& address);
    void get_state(uint64_t &height,  uint64_t &rate, std::string& difficulty, bool& is_mining);
    bool get_block_header(chain::header& block_header, const std::string& para);

    static int get_lock_heights_index(uint64_t height);
    static uint64_t calculate_block_subsidy(uint64_t height, bool is_testnet);
    static uint64_t calculate_lockblock_reward(uint64_t lcok_heights, uint64_t num);

private:
    void work(const wallet::payment_address pay_address);
    block_ptr create_new_block(const wallet::payment_address& pay_addres);
    block_ptr create_new_block_pos(const std::string account, const std::string passwd, const wallet::payment_address& pay_addres);
    unsigned int get_adjust_time(uint64_t height) const;
    unsigned int get_median_time_past(uint64_t height) const;
    bool get_transaction(std::vector<transaction_ptr>&, previous_out_map_t&, tx_fee_map_t&) const;
    bool get_block_transactions(
        uint64_t last_height, std::vector<transaction_ptr>& txs, std::vector<transaction_ptr>& reward_txs, 
        uint64_t& total_fee, unsigned int& total_tx_sig_length);
    uint64_t store_block(block_ptr block);
    uint64_t get_height() const;
    bool get_input_etp(const transaction&, const std::vector<transaction_ptr>&, uint64_t&, previous_out_map_t&) const ;
    bool is_stop_miner(uint64_t block_height) const;
    unsigned int get_tx_sign_length(transaction_ptr tx);

private:
    p2p_node& node_;
    std::shared_ptr<boost::thread> thread_;
    mutable state state_;
    uint16_t new_block_number_;
    uint16_t new_block_limit_;

    block_ptr new_block_;
    wallet::payment_address pay_address_;
    const blockchain::settings& setting_;

    bool isStaking_;
    std::string account_;
    std::string passwd_;
};

}
}

#endif
