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

#ifndef MVS_CONSENSUS_WITNESS_HPP
#define MVS_CONSENSUS_WITNESS_HPP

#include <vector>
#include <metaverse/bitcoin.hpp>

// forward declaration
namespace libbitcoin {
    namespace node {
        class p2p_node;
    } // node
    namespace blockchain {
        class settings;
        class validate_block;
    } // blockchain
    namespace wallet {
        class payment_address;
    } // wallet
} // libbitcoin

namespace libbitcoin {
namespace consensus {

class witness
{
public:
    using p2p_node = libbitcoin::node::p2p_node;
    using settings = blockchain::settings;
    using validate_block = blockchain::validate_block;

    using public_key_t = data_chunk;
    using witness_id = data_chunk;
    using list = std::vector<witness_id>;
    using iterator = list::iterator;
    using const_iterator = list::const_iterator;

    static uint64_t witness_enable_height;
    static uint32_t witness_number;
    static uint32_t epoch_cycle_height;
    static uint32_t register_witness_lock_height;
    static uint64_t witness_lock_threshold;
    static uint32_t vote_maturity;
    static uint64_t witness_register_enable_height;

    static const uint32_t max_candidate_count;
    static const uint32_t witness_register_fee;
    static const std::string witness_registry_did;

public:
    ~witness();

    // singleton
    static witness& create(p2p_node& node);
    static witness& get();

    const witness_id& get_witness(uint32_t slot) const;

    // return a copy list
    list get_witness_list() const;
    void swap_witness_list(list&);

    std::string show_list() const;
    static std::string show_list(const list& witness_list);

    bool is_witness(const witness_id& id) const;

    static std::shared_ptr<witness::list> get_block_witnesses(const chain::block& block);
    std::shared_ptr<witness::list> get_block_witnesses(uint64_t height) const;
    std::shared_ptr<std::vector<std::string>> get_inactive_witnesses(uint64_t height) const;

    // generate a new epoch witness list
    bool calc_witness_list(uint64_t height);
    bool calc_witness_list(list& witness_list, uint64_t height) const;
    bool init_witness_list();
    bool update_witness_list(uint64_t height);
    bool update_witness_list(const chain::block& block);
    chain::output create_witness_vote_result(uint64_t height);
    bool add_witness_vote_result(chain::transaction& coinbase_tx, uint64_t block_height);
    static bool is_vote_result_output(const chain::output&);

    uint32_t get_slot_num(const witness_id& id) const;
    uint32_t calc_slot_num(uint64_t height) const;
    size_t get_witness_number();

    std::string witness_to_address(const witness_id& witness) const;

    static public_key_t witness_to_public_key(const witness_id& id);
    static std::string witness_to_string(const witness_id& id);
    static std::string endorse_to_string(const endorsement& endorse);
    static witness_id to_witness_id(const public_key_t& public_key);
    static std::string to_string(const public_key_t& public_key);

    static bool is_witness_enabled(uint64_t height);
    static bool is_dpos_enabled();
    static uint64_t get_height_in_epoch(uint64_t height);
    static uint64_t get_epoch_begin_height(uint64_t height);
    static uint64_t get_round_begin_height(uint64_t height);

    static bool is_begin_of_epoch(uint64_t height);
    static bool is_in_same_epoch(uint64_t height1, uint64_t height2);

    chain::block::ptr get_epoch_begin_block(uint64_t height) const;

    // signature
    static bool sign(endorsement& out, const ec_secret& secret, const chain::header& h);
    static bool verify_sign(const endorsement& out, const public_key_t& public_key, const chain::header& h);
    bool verify_signer(const public_key_t& public_key, uint64_t height) const;
    bool verify_signer(uint32_t witness_slot_num, uint64_t height) const;
    bool verify_witness_list(const chain::block& block, list& witness_list) const;

    static u256 calc_mixhash(const list& witness_list);

    void set_validate_block(const validate_block*);
    bool get_header(chain::header& out_header, uint64_t height) const;
    uint64_t get_last_height();

private:
    witness(p2p_node& node);

    static void init(p2p_node& node);
    static bool exists(const list&, const witness_id&);
    static const_iterator finds(const list&, const witness_id&);
    static iterator finds(list&, const witness_id&);

    chain::block::ptr fetch_block(uint64_t height) const;

private:
    static witness* instance_;
    p2p_node& node_;
    const settings& setting_;
    list witness_list_;
    const validate_block* validate_block_;
    mutable upgrade_mutex mutex_;
};

struct witness_with_validate_block_context
{
    using validate_block = blockchain::validate_block;
    witness& witness_;
    witness_with_validate_block_context(witness& w, const validate_block* v);
    ~witness_with_validate_block_context();
};

} // consensus
} // libbitcoin

#endif
