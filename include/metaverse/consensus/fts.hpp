/**
 * Copyright (c) 2019-2020 metaverse core developers (see MVS-AUTHORS)
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
 * A stakeholder in the Merkle tree. Each stakeholder has an address,
 * and controls an amount of coins.
 */
class fts_stake_holder
{
public:
    typedef std::shared_ptr<fts_stake_holder> ptr;
    typedef std::vector<fts_stake_holder> list;
    typedef std::vector<ptr> ptr_list;

    fts_stake_holder();
    fts_stake_holder(const std::string& address, uint64_t stake);
    fts_stake_holder(const fts_stake_holder& other);

    bool operator==(const fts_stake_holder& other) const;

    void set_stake(uint64_t stake);
    void set_address(const std::string& address);

    uint64_t stake() const;
    std::string address() const;

    std::string to_string() const;

    static std::shared_ptr<ptr_list> convert(const list& stakeholders);

  private:
    std::string address_;
    uint64_t stake_;
};

/**
 * Node of follow-the-satoshi merkle tree
 */
class fts_node
{
public:
    typedef std::vector<fts_node> list;
    typedef std::shared_ptr<fts_node> ptr;

    fts_node(const fts_node::ptr& left, const fts_node::ptr& right);
    fts_node(const fts_stake_holder::ptr& stakeholder);

    fts_node::ptr left() const;
    fts_node::ptr right() const;
    fts_stake_holder::ptr stake_holder() const;
    hash_digest hash() const;

    uint64_t stake() const;
    bool is_leaf() const;

    std::string to_string() const;

private :
    fts_node::ptr left_;
    fts_node::ptr right_;
    fts_stake_holder::ptr stake_holder_;
    hash_digest hash_;
};

/**
 * Follow-the-satoshi algorithm
 * The edges the Merkle tree are labelled with the amount of coins in the left
 * and right subtree respectively. Given a psuedo-random number generator,
 * one can randomly select a stakeholder from the tree, weighted by the amount
 * of coins they own, by traversing the tree down to a leaf node, containing one
 * of the stakeholders. Each stakeholder controls a number of coins
 * and a private key used to sign blocks.
 */
class fts
{
public:

    // select count items from stakeholders.
    static std::shared_ptr<fts_stake_holder::ptr_list> select_by_fts(
        const fts_stake_holder::ptr_list& stakeholders,
        uint32_t seed, uint32_t count);

    // build FTS Merkle tree with stakeholders
    static fts_node::ptr build_merkle_tree(
        const fts_stake_holder::ptr_list& stakeholders);

    // select one item from FTS Merkle tree.
    static fts_node::ptr select_by_fts(const fts_node::ptr& merkle_tree, uint32_t seed);

    // target_hash is mixed hash of merkle root and stake leaf node.
    static bool verify(const fts_node::ptr& merkle_tree, uint32_t seed, const hash_digest& target_hash);

    static hash_digest to_hash(const fts_node::ptr& left, const fts_node::ptr& right);
    static hash_digest to_hash(const fts_stake_holder& stakeholder);

    static void test();
};

} // consensus
} // libbitcoin

#endif
