/**
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
#include <metaverse/consensus/fts.hpp>
#include <metaverse/macros_define.hpp>
#include <metaverse/node/p2p_node.hpp>
#include <metaverse/blockchain/settings.hpp>
#include <future>
#include <random>

#define LOG_HEADER "FTS"

namespace libbitcoin {
namespace consensus {

//==============================================================================
// fts_proof_entry
//==============================================================================
fts_proof_entry::fts_proof_entry()
    : left_stake_(0)
    , right_stake_(0)
{}

fts_proof_entry::fts_proof_entry(const hash_digest& hash, uint64_t left_stake, uint64_t right_stake)
    : hash_(hash)
    , left_stake_(left_stake)
    , right_stake_(right_stake)
{
}

fts_proof_entry::fts_proof_entry(const fts_proof_entry& other)
{
    hash_ = other.hash_;
    left_stake_ = other.left_stake_;
    right_stake_ = other.right_stake_;
}

void fts_proof_entry::set_hash(const hash_digest& hash)
{
    hash_ = hash;
}

void fts_proof_entry::set_left_stake(const uint64_t& stake)
{
    left_stake_ = stake;
}

void fts_proof_entry::set_right_stake(const uint64_t& stake)
{
    right_stake_ = stake;
}

hash_digest fts_proof_entry::hash() const
{
    return hash_;
}

uint64_t fts_proof_entry::left_stake() const
{
    return left_stake_;
}

uint64_t fts_proof_entry::right_stake() const
{
    return right_stake_;
}

std::string fts_proof_entry::to_string() const
{
    std::ostringstream ss;
    ss << "fts_proof_entry: {hash: " << encode_hash(hash_);
    ss << ", left: " << left_stake_;
    ss << ", right: " << right_stake_ << "}";
    return ss.str();
}


//==============================================================================
// fts_stake_holder
//==============================================================================
fts_stake_holder::fts_stake_holder()
    : address_("")
    , stake_(0)
{}

fts_stake_holder::fts_stake_holder(const std::string& address, uint64_t stake)
    : address_(address)
    , stake_(stake)
{}

fts_stake_holder::fts_stake_holder(const fts_stake_holder& other)
{
    address_ = other.address_;
    stake_ = other.stake_;
}

void fts_stake_holder::set_stake(uint64_t stake)
{
    stake_ = stake;
}

void fts_stake_holder::set_address(const std::string& address)
{
    address_ = address;
}

uint64_t fts_stake_holder::stake() const
{
    return stake_;
}

std::string fts_stake_holder::address() const
{
    return address_;
}

bool fts_stake_holder::is_empty() const
{
    return (address_.empty() && stake_ == 0);
}

std::string fts_stake_holder::to_string() const
{
    std::ostringstream ss;
    ss << "fts_stake_holder: {address: " << address_;
    ss << ", stake: " << stake_ << "}";
    return ss.str();
}


//==============================================================================
// fts_node
//==============================================================================
static hash_digest to_hash(fts_node::ptr left, fts_node::ptr right)
{
    BITCOIN_ASSERT(left != nullptr);
    BITCOIN_ASSERT(right != nullptr);

    // Join both current hashes together (concatenate).
    data_chunk concat_data;
    data_sink concat_stream(concat_data);
    ostream_writer concat_sink(concat_stream);
    concat_sink.write_hash(left->hash());
    concat_sink.write_hash(right->hash());
    concat_sink.write_8_bytes_little_endian(left->stake());
    concat_sink.write_8_bytes_little_endian(right->stake());
    concat_stream.flush();

    // Hash both of the hashes and stakes.
    return bitcoin_hash(concat_data);
}

static hash_digest to_hash(const fts_stake_holder& stakeholder)
{
    // Join both address and stake.
    data_chunk concat_data;
    data_sink concat_stream(concat_data);
    ostream_writer concat_sink(concat_stream);
    concat_sink.write_string(stakeholder.address());
    concat_sink.write_8_bytes_little_endian(stakeholder.stake());
    concat_stream.flush();

    // Hash both of the address and stake.
    return bitcoin_hash(concat_data);
}

fts_node::fts_node(fts_node::ptr left, fts_node::ptr right)
    : left_(left)
    , right_(right)
    , hash_(to_hash(left, right))
{
    log::info("fts_node")
        << "\n >> new node: " << encode_hash(hash_)
        << "\n        left: " << encode_hash(left_->hash())
        << "\n       right: " << encode_hash(right->hash());
}

fts_node::fts_node(const fts_stake_holder& stakeholder)
    : left_(nullptr)
    , right_(nullptr)
    , stake_holder_(stakeholder)
    , hash_(to_hash(stake_holder_))
{
    log::info("fts_node")
            << "\n >> new node: " << encode_hash(hash_)
            << "\n     address: " << stake_holder_.address()
            << "\n       stake: " << stake_holder_.stake();
}

fts_node::ptr fts_node::left() const
{
    return left_;
}

fts_node::ptr fts_node::right() const
{
    return right_;
}

fts_stake_holder fts_node::stake_holder() const
{
    return stake_holder_;
}

hash_digest fts_node::hash() const
{
    return hash_;
}

uint64_t fts_node::stake() const
{
    if (is_leaf()) {
        return stake_holder_.stake();
    }

    return (left_->stake() + right_->stake());
}

bool fts_node::is_leaf() const
{
    return (left_ == nullptr && right_ == nullptr);
}

std::string fts_node::to_string() const
{
    std::ostringstream ss;
    ss << "fts_node: {hash: " << encode_hash(hash_);
    ss << ", stake: " << stake() << "}";
    return ss.str();
}

//==============================================================================
// fts
//==============================================================================
fts_node::ptr fts::build_merkle_tree(const fts_stake_holder::list& stakeholders)
{
    if (stakeholders.empty()) {
        return nullptr;
    }

    typedef std::vector<fts_node::ptr> node_vec;
    typedef std::shared_ptr<node_vec> node_vec_ptr;
    node_vec_ptr merkle = std::make_shared<node_vec>(stakeholders.size());
    for (auto holder : stakeholders) {
        fts_node node(holder);
        auto node_ptr = std::make_shared<fts_node>(node);
        merkle->push_back(node_ptr);
    }

    log::info("build_merkle_tree")
            << "stakeholders size: " << stakeholders.size();
    log::info("build_merkle_tree")
            << "nodes size: " << merkle->size();

    for (auto it = merkle->begin(); it != merkle->end(); it ++) {
        // Hash both of the hashes.
        auto node = *it;
        log::info("build_merkle_tree")
            << "\n    node: " << node->to_string();
    }

    // // While there is more than 1 hash in the list, keep looping...
    // while (merkle->size() > 1) {
    //     // If number of hashes is odd, duplicate last hash in the list.
    //     if (merkle->size() % 2 != 0) {
    //         merkle->push_back(merkle->back());
    //     }

    //     // List size is now even.
    //     BITCOIN_ASSERT(merkle->size() % 2 == 0);

    //     // New hash list.
    //     node_vec_ptr new_merkle = std::make_shared<node_vec>();

    //     // Loop through hashes 2 at a time.
    //     for (auto it = merkle->begin(); it != merkle->end(); it += 2) {
    //         // Hash both of the hashes.
    //         auto left = *it;
    //         auto right = *(it + 1);
    //         log::info("build_merkle_tree")
    //             << "\n    left: " << left->to_string()
    //             << "\n   right: " << right->to_string();

    //         auto new_root = std::make_shared<fts_node>(left, right);

    //         // Add this to the new list.
    //         new_merkle->push_back(new_root);
    //     }

    //     // This is the new list.
    //     merkle = new_merkle;
    // }

    // Finally we end up with a single item.
    return *merkle->begin();
}

fts_node::ptr fts::select_by_fts(fts_node::ptr merkle_tree, uint32_t seed)
{
    if (merkle_tree == nullptr) {
        return false;
    }

    fts_node::ptr node = merkle_tree;

    // Seeds the pseudo-random number generator
    std::default_random_engine eng(seed);
    std::uniform_int_distribution<uint64_t> urd(0, node->stake());

    while (!node->is_leaf()) {
        auto left = node->left();
        auto right = node->right();
        uint64_t r = urd(eng) % node->stake();
        if (r <= left->stake()) {
            node = left;
        }
        else {
            node = right;
        }
    }

    BITCOIN_ASSERT(node->is_leaf());
    return node;
}

bool fts::verify(fts_node::ptr merkle_tree, uint32_t seed, const hash_digest& stake_hash)
{
    auto selected = select_by_fts(merkle_tree, seed);
    if (selected != nullptr) {
        return selected->hash() == stake_hash;
    }

    return false;
}


}
}
