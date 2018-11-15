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

#include <set>
#include <metaverse/bitcoin.hpp>

// forward declaration
namespace libbitcoin {
    namespace node {
        class p2p_node;
    } // node
    namespace blockchain {
        class settings;
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
    using public_key_t = data_chunk;
    using witness_id = public_key_t;
    using list = std::vector<witness_id>;
    using iterator = list::iterator;
    using const_iterator = list::const_iterator;

    static uint32_t pow_check_point_height;
    static uint64_t witness_enable_height;
    static uint32_t witess_number;
    static uint32_t epoch_cycle_height;
    static uint32_t vote_maturity;

public:
    ~witness();

    // singleton
    static witness& create(p2p_node& node);
    static witness& get();

    bool is_witness_prepared() const;

    // return a copy list
    list get_witness_list() const;
    list get_candidate_list() const;
    void swap_witness_list(list&);
    void swap_candidate_list(list&);
    std::string show_list() const;

    bool is_witness(const witness_id& id) const;

    // register as a candidate
    bool register_witness(const witness_id& id);
    bool unregister_witness(const witness_id& id);

    // generate a new epoch witness list
    bool calc_witness_list(uint64_t height);
    bool update_witness_list(uint64_t height);
    bool update_witness_list(const chain::block& block);
    chain::output create_witness_vote_result(uint64_t height);
    chain::block::ptr fetch_vote_result_block(uint64_t height);

    uint32_t get_slot_num(const witness_id& id) const;

    static bool is_witness_enabled(uint64_t height);
    static uint64_t get_height_in_epoch(uint64_t height);
    static uint64_t get_vote_result_height(uint64_t height);

    static bool is_begin_of_epoch(uint64_t height);
    static bool is_between_vote_maturity_interval(uint64_t height);

    // signature
    static bool sign(endorsement& out, const ec_secret& secret, const chain::header& h);
    static bool verify_sign(const endorsement& out, const public_key_t& public_key, const chain::header& h);
    bool verify_signer(const public_key_t& public_key, const chain::block& block, const chain::header& prev_header) const;
    bool verify_signer(uint32_t witness_slot_num, const chain::block& block, const chain::header& prev_header) const;
    bool verify_vote_result(const chain::block& block) const;

private:
    witness(p2p_node& node);
    static void init(p2p_node& node);
    static bool exists(const list&, const witness_id&);
    static const_iterator finds(const list&, const witness_id&);
    static iterator finds(list&, const witness_id&);

private:
    static witness* instance_;
    p2p_node& node_;
    const settings& setting_;
    list witness_list_;
    list candidate_list_;
    mutable upgrade_mutex mutex_;
};

} // consensus
} // libbitcoin

#endif
