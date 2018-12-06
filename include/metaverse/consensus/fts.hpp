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

#ifndef MVS_CONSENSUS_FTS_HPP
#define MVS_CONSENSUS_FTS_HPP

#include <vector>
#include <metaverse/bitcoin.hpp>

namespace libbitcoin {
namespace consensus {

/**
 * A class containing an entry (hash, left_stake, right_stake) in a Merkle proof, where
 * "left_stake" is the number of coins in the left subtree, "right_stake" is the number
 * of coins the right subtree and "hash" is the Merkle hash
 * H(left hash | right hash | left_stake | right_stake).
 */
class fts_proof_entry
{
public:
    typedef std::vector<fts_proof_entry> list;
    typedef std::shared_ptr<fts_proof_entry> ptr;

    fts_proof_entry();
    fts_proof_entry(const hash_digest& hash, uint64_t left_stake, uint64_t right_stake);
    fts_proof_entry(const fts_proof_entry& other);

    void set_hash(const hash_digest& hash);
    void set_left_stake(const uint64_t& stake);
    void set_right_stake(const uint64_t& stake);

    hash_digest hash() const;
    uint64_t left_stake() const;
    uint64_t right_stake() const;

    std::string to_string() const;

private:
    hash_digest hash_;
    uint64_t left_stake_;
    uint64_t right_stake_;
};

/**
 * A stakeholder in the Merkle tree. Each stakeholder has an address,
 * and controls an amount of coins.
 */
class fts_stake_holder
{
public:
    typedef std::vector<fts_stake_holder> list;
    typedef std::shared_ptr<fts_stake_holder> ptr;

    fts_stake_holder();
    fts_stake_holder(const std::string& address, uint64_t stake);
    fts_stake_holder(const fts_stake_holder& other);

    void set_stake(uint64_t stake);
    void set_address(const std::string& address);

    uint64_t stake() const;
    std::string address() const;
    bool is_empty() const;

    std::string to_string() const;

  private:
    std::string address_;
    uint64_t stake_;
};

class fts_node
{
public:
    typedef std::vector<fts_node> list;
    typedef std::shared_ptr<fts_node> ptr;

    fts_node(fts_node::ptr left, fts_node::ptr right);
    fts_node(fts_stake_holder stakeholder);

    fts_node::ptr left() const;
    fts_node::ptr right() const;
    fts_stake_holder stake_holder() const;
    hash_digest hash() const;

    uint64_t stake() const;
    bool is_leaf() const;

    std::string to_string() const;

  private:
    fts_node::ptr left_;
    fts_node::ptr right_;
    fts_stake_holder stake_holder_;
    hash_digest hash_;
};

class fts
{
public:
    static fts_node::ptr build_merkle_tree(const fts_stake_holder::list& stakeholders);
    static fts_node::ptr select_by_fts(fts_node::ptr merkle_tree, uint32_t seed);
    static bool verify(fts_node::ptr merkle_tree, uint32_t seed, const hash_digest& stake_hash);
private:

private:
};

} // consensus
} // libbitcoin

#endif
