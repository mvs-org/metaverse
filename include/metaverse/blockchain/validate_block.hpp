/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse.
 *
 * metaverse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MVS_BLOCKCHAIN_VALIDATE_HPP
#define MVS_BLOCKCHAIN_VALIDATE_HPP

#include <cstddef>
#include <cstdint>
#include <system_error>
#include <vector>
#include <metaverse/bitcoin.hpp>
#include <metaverse/blockchain/define.hpp>

namespace libbitcoin {
namespace blockchain {

class block_chain_impl;

// Max block size (1000000 bytes).
constexpr uint32_t max_block_size = 1000000;

// Maximum signature operations per block (20000).
constexpr uint32_t max_block_script_sigops = max_block_size / 50;


// TODO: this is not an interface, collapse with validate_block_impl.
/// This class is not thread safe.
class BCB_API validate_block
{
public:
    code check_block(blockchain::block_chain_impl& chain) const;
    code accept_block() const;
    code connect_block(hash_digest& err_tx, blockchain::block_chain_impl& chain) const;

    /// Required to call before calling accept_block or connect_block.
    void initialize_context();
    static bool script_hash_signature_operations_count(uint64_t& out_count, const chain::script& output_script, const chain::script& input_script);

    bool get_transaction(const hash_digest& tx_hash, chain::transaction& prev_tx, uint64_t& prev_height) const;
    bool get_header(chain::header& out_header, uint64_t height) const;

    virtual std::string get_did_from_address_consider_orphan_chain(const std::string& address, const std::string& did_symbol) const = 0;
    virtual bool is_did_match_address_in_orphan_chain(const std::string& symbol, const std::string& address) const = 0;
    virtual bool is_did_in_orphan_chain(const std::string& symbol) const = 0;
    virtual bool is_asset_in_orphan_chain(const std::string& symbol) const = 0;
    virtual bool is_asset_cert_in_orphan_chain(const std::string& symbol, asset_cert_type cert_type) const = 0;
    virtual bool is_asset_mit_in_orphan_chain(const std::string& symbol) const = 0;

    virtual uint64_t get_fork_index() const { return max_uint64; }
    const uint64_t get_height() const {return height_;}
    virtual uint64_t median_time_past() const = 0;

    virtual chain::block::ptr fetch_full_block(uint64_t height) const {return nullptr;}

    code check_coinbase(const chain::header& prev_header, bool check_genesis_tx) const;

protected:
    typedef std::vector<uint8_t> versions;
    typedef std::function<bool()> stopped_callback;

    validate_block(uint64_t height, const chain::block& block,
        bool testnet, const config::checkpoint::list& checks,
        stopped_callback stop_callback);

    virtual bool check_get_coinage_reward_transaction(const chain::transaction& coinage_reward_coinbase, const chain::output& tx) const = 0;
    virtual u256 previous_block_bits() const = 0;
    virtual uint64_t actual_time_span(uint64_t interval) const = 0;
    virtual versions preceding_block_versions(uint64_t count) const = 0;
    virtual chain::header fetch_block(uint64_t fetch_height) const = 0;
    virtual chain::header::ptr get_last_block_header(const chain::header& parent_header, uint32_t version) const = 0;
    virtual bool transaction_exists(const hash_digest& tx_hash) const = 0;
    virtual bool fetch_transaction(chain::transaction& tx, uint64_t& tx_height,
        const hash_digest& tx_hash) const = 0;
    virtual bool is_output_spent(const chain::output_point& outpoint) const = 0;
    virtual bool is_output_spent(const chain::output_point& previous_output,
        uint64_t index_in_parent, uint64_t input_index) const = 0;

    // These have default implementations that can be overriden.
    virtual bool connect_input(uint64_t index_in_parent,
        const chain::transaction& current_tx, uint64_t input_index,
        uint64_t& value_in, uint64_t& total_sigops) const;
    virtual bool validate_inputs(const chain::transaction& tx,
        uint64_t index_in_parent, uint64_t& value_in,
        uint64_t& total_sigops) const;

    // These are protected virtual for testability.
    bool stopped() const;
    virtual bool is_valid_version() const;
    virtual bool is_active(chain::script_context flag) const;
    bool is_spent_duplicate(const chain::transaction& tx) const;
    bool is_valid_time_stamp(uint32_t timestamp) const;
    bool is_valid_time_stamp_new(uint32_t timestamp) const;
    bool check_time_stamp(uint32_t timestamp, const asio::seconds& window) const;
    u256 work_required(bool is_testnet) const;

    bool check_block_signature(blockchain::block_chain_impl& chain) const;

    virtual bool verify_stake(const chain::block& block) const = 0;
    virtual bool is_coin_stake(const chain::block& block) const = 0;


    virtual bool check_work(const chain::block& block) const = 0;

    static bool is_distinct_tx_set(const chain::transaction::list& txs);
    static bool is_valid_coinbase_height(uint64_t height, const chain::block& block, uint64_t index);

    virtual bool can_use_dpos(uint64_t height) const = 0;
    virtual uint64_t get_pow_height_before_dpos(uint64_t height) const = 0;
    virtual chain::header::ptr get_prev_block_header(uint64_t height, chain::block_version ver) const = 0;

private:
    bool testnet_;
    const uint64_t height_;
    uint32_t activations_;
    const chain::block& current_block_;
    const config::checkpoint::list& checkpoints_;
    const stopped_callback stop_callback_;
};

} // namespace blockchain
} // namespace libbitcoin

#endif
