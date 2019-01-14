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
#ifndef MVS_BLOCKCHAIN_IMPL_VALIDATE_BLOCK_H
#define MVS_BLOCKCHAIN_IMPL_VALIDATE_BLOCK_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include <metaverse/bitcoin.hpp>
#include <metaverse/blockchain/validate_block.hpp>
#include <metaverse/blockchain/block_detail.hpp>

namespace libbitcoin {
namespace blockchain {

class block_chain_impl;

/// This class is not thread safe.
class BCB_API validate_block_impl
  : public validate_block
{
public:
    validate_block_impl(block_chain_impl& chain, uint64_t fork_index,
        const block_detail::list& orphan_chain, uint64_t orphan_index,
        uint64_t height, const chain::block& block, bool testnet,
        const config::checkpoint::list& checkpoints,
        stopped_callback stopped);

    virtual bool verify_stake(const chain::block& block) const override;
    virtual bool is_coin_stake(const chain::block& block) const override;

    virtual bool check_work(const chain::block& block) const override;
    virtual bool check_get_coinage_reward_transaction(
        const chain::transaction& coinage_reward_coinbase, const chain::output& output) const override;

    virtual std::string get_did_from_address_consider_orphan_chain(const std::string& address, const std::string& did_symbol) const override;
    virtual bool is_did_match_address_in_orphan_chain(const std::string& symbol, const std::string& address) const override;
    virtual bool is_did_in_orphan_chain(const std::string& symbol) const override;
    virtual bool is_asset_in_orphan_chain(const std::string& symbol) const override;
    virtual bool is_asset_cert_in_orphan_chain(const std::string& symbol, asset_cert_type cert_type) const override;
    virtual bool is_asset_mit_in_orphan_chain(const std::string& symbol) const override;

    virtual uint64_t get_fork_index() const override { return fork_index_; }
    uint64_t median_time_past() const override;

    virtual chain::block::ptr fetch_full_block(uint64_t height) const override;

protected:
    u256 previous_block_bits() const override;
    uint64_t actual_time_span(uint64_t interval) const override;
    versions preceding_block_versions(uint64_t maximum) const override;
    chain::header fetch_block(uint64_t fetch_height) const override;
    chain::header::ptr get_last_block_header(const chain::header& parent_header, uint32_t version) const override;
    bool fetch_transaction(chain::transaction& tx, uint64_t& tx_height,
        const hash_digest& tx_hash) const override;
    bool is_output_spent(const chain::output_point& outpoint) const override;
    bool is_output_spent(const chain::output_point& previous_output,
        uint64_t index_in_parent, uint64_t input_index) const override;
    bool transaction_exists(const hash_digest& tx_hash) const override;

    virtual bool can_use_dpos(uint64_t height) const override;
    virtual bool check_max_successive_height(uint64_t height, chain::block_version version) const override;
    virtual chain::header::ptr get_prev_block_header(
        uint64_t height, chain::block_version ver, bool same_version=true) const override;

private:
    bool fetch_orphan_transaction(chain::transaction& tx,
        uint64_t& previous_height, const hash_digest& tx_hash) const;
    bool orphan_is_spent(const chain::output_point& previous_output,
        uint64_t skip_tx, uint64_t skip_input) const;

    block_chain_impl& chain_;
    uint64_t height_;
    uint64_t fork_index_;
    uint64_t orphan_index_;
    const block_detail::list& orphan_chain_;
};

} // namespace blockchain
} // namespace libbitcoin

#endif
