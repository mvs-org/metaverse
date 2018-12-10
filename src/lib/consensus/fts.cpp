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
#include <ctime>
#include <random>
#include <algorithm>

#define LOG_HEADER "FTS"

namespace libbitcoin {
namespace consensus {


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

bool fts_stake_holder::operator==(const fts_stake_holder& other) const
{
    return (address_ == other.address_ && stake_ == other.stake_);
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

std::string fts_stake_holder::to_string() const
{
    std::ostringstream ss;
    ss << "fts_stake_holder: {address: " << address_;
    ss << ", stake: " << stake_ << "}";
    return ss.str();
}

std::shared_ptr<fts_stake_holder::ptr_list> fts_stake_holder::convert(const fts_stake_holder::list& stake_holders)
{
    auto temp = std::make_shared<fts_stake_holder::ptr_list>();
    temp->resize(stake_holders.size());
    size_t i = 0;
    for (auto& holder : stake_holders) {
        (*temp)[i++] = std::make_shared<fts_stake_holder>(holder);
    }

    return temp;
}

//==============================================================================
// fts_node
//==============================================================================

fts_node::fts_node(const fts_node::ptr& left, const fts_node::ptr& right)
    : left_(left)
    , right_(right)
    , stake_holder_(nullptr)
    , hash_(fts::to_hash(left, right))
{
    BITCOIN_ASSERT(left_ != nullptr && right_ != nullptr);
}

fts_node::fts_node(const fts_stake_holder::ptr& stakeholder)
    : left_(nullptr)
    , right_(nullptr)
    , stake_holder_(stakeholder)
    , hash_(fts::to_hash(*stake_holder_))
{
    BITCOIN_ASSERT(stake_holder_ != nullptr);
}

fts_node::ptr fts_node::left() const
{
    return left_;
}

fts_node::ptr fts_node::right() const
{
    return right_;
}

fts_stake_holder::ptr fts_node::stake_holder() const
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
        return stake_holder_->stake();
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

hash_digest fts::to_hash(const fts_node::ptr& left, const fts_node::ptr& right)
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

hash_digest fts::to_hash(const fts_stake_holder& stakeholder)
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

std::shared_ptr<fts_stake_holder::ptr_list> fts::select_by_fts(
    const fts_stake_holder::ptr_list& stake_holders,
    uint32_t seed, uint32_t count)
{
    auto size = stake_holders.size();
    if (size == 0) {
        return nullptr;
    }

    auto temp = std::make_shared<fts_stake_holder::ptr_list>(stake_holders.size());
    std::copy(stake_holders.begin(), stake_holders.end(), temp->begin());

    if (size <= count) {
        return temp;
    }

    auto result = std::make_shared<fts_stake_holder::ptr_list>();

    while (result->size() < count && temp->size() > 1) {
        auto root = fts::build_merkle_tree(*temp);
        BITCOIN_ASSERT(root != nullptr);
        auto choosen = fts::select_by_fts(root, seed);
        BITCOIN_ASSERT(choosen != nullptr && choosen->is_leaf());

        auto holder = choosen->stake_holder();
        result->push_back(holder);
        temp->erase(std::remove(temp->begin(), temp->end(), holder));
    }

    auto miss = count - result->size();
    if (miss > 0) {
        for (auto& holder : *temp) {
            result->push_back(holder);

            if (--miss == 0) {
                break;
            }
        }
    }

    return result;
}

fts_node::ptr fts::build_merkle_tree(const fts_stake_holder::ptr_list& stake_holders)
{
    if (stake_holders.empty()) {
        return nullptr;
    }

    const size_t size = stake_holders.size();
    std::vector<fts_node::ptr> tree(size * 2);

    for (size_t i = 0; i < size; i++) {
        auto& holder = stake_holders[i];
        tree[size + i] = std::make_shared<fts_node>(holder);
    }

    for (size_t i = size - 1; i > 0; i--) {
        fts_node::ptr left = tree[i * 2];
        fts_node::ptr right = tree[i * 2 + 1];
        tree[i] = std::make_shared<fts_node>(left, right);
    }

    BITCOIN_ASSERT(tree.size() > 1);
    return tree[1];
}

fts_node::ptr fts::select_by_fts(const fts_node::ptr& merkle_tree, uint32_t seed)
{
    if (merkle_tree == nullptr) {
        return nullptr;
    }

    fts_node::ptr node = merkle_tree;

    // Seeds the pseudo-random number generator
    std::default_random_engine eng(seed);
    std::uniform_int_distribution<uint64_t> urd(0, node->stake());

    while (!node->is_leaf()) {
        auto left = node->left();
        auto right = node->right();
        uint64_t r = urd(eng) % node->stake();
        if (r < left->stake()) {
            node = left;
        }
        else {
            node = right;
        }
    }

    BITCOIN_ASSERT(node->is_leaf());
    return node;
}

bool fts::verify(const fts_node::ptr& merkle_tree, uint32_t seed, const hash_digest& target_hash)
{
    if (merkle_tree == nullptr) {
        return false;
    }

    auto selected = select_by_fts(merkle_tree, seed);
    if (selected != nullptr) {
        auto mix_hash = fts::to_hash(merkle_tree, selected);
        return mix_hash == target_hash;
    }

    return false;
}

void fts::test()
{
    const uint32_t seed = std::time(nullptr);

    fts_stake_holder::list stake_data = {
        {"MDdET3ybWc2cGEXXxBcjtXNCcmzJe48bhc", 333000000},
        {"MQA3r2AVy9TLzoYwdyCmT2roCqTHVRk2Tj", 331000000},
        {"MWLwUrmgdGGADJmQ9nsCSHDGz6BZd1aGhw", 333000000},
        {"MUFhTGxWE2zciFYY4oQ4NJqNnz6u4Yi1dy", 8000000},
        {"MPgsYGbKfhLRptHLjHvNU2B2GumqTYSdmp", 9000000},
        {"MQibP3A6VNGqR5ZbKpkyKrU52zA8nQDZt8", 1000000},
        {"MCwCVvrQ94fHg2hjY6JYEWiXwedUa6uxFF", 2000000},
        {"M8LZMA7vVCsWrWPsZdVy4Tac5WxrPKNZTh", 7000000},
        {"MV1VWVC7NiJ6BmZPXooiamZcp51SxMUFv3", 12000000},
        {"MAGjtX89zwjXtBTeKQc1tHSatWd2ivXm64", 26000000},
        {"MUMsvrkdm3yaJDBhYq9LT9UeynKhh1fhRd", 36000000},
        {"MVgazXx68NQfMb6Dm5Xbx7HqXxUoqt6Ab9", 28000000},
        {"MJuQPM6TuewrhPsmmAWab1qep2nw9fpU5X", 29000000},
        {"MQ4Ygm2nieCM6J1EkdaHjLNokRLBoG2SCT", 33000000},
        {"MLVC5FjVrx3MKt8UmuQxSVBQZa9uD4P4ZZ", 22300000},
        {"MCiggAFxy76WRRQQwrbSfc4oWZLMZiW2mE", 11200000},
        {"MJZLuKx6EeqggDB645kiwEkRwZ9qSfnqkY", 23600000},
        {"MPNT1Z8s8SkMh4kHSgxE2MdArMoAgHRN7o", 4900000},
        {"MHuLk8CKrPFB68WcQDb4o8JPVX8ZT3p7jq", 8900000},
        {"MBV6pQbHaRFUSGCo3oxUMy7w2dVDt2B9sN", 6700000},
        {"MGG7nhM6aKXzFQK4foWEsCE7UC79q3vCcs", 2200000},
        {"MHsukoRTNjuW6FyCAmyzNwwt2geWbYN1jh", 6800000},
        {"MFBv4HW8PBxY9Cz1TpqCA9aN5zWixqwvKy", 2700000},
        {"MNyvrdXC6CNjEpKu4nJgP35mT5f38nvpp4", 3700000},
        {"MVYAmc2RUfaGPFj4B8kLvEKv38DtZ2sGs9", 3800000},
        {"MUDPXwb3nAoPChrYWGeSBfTK4p8DVg5Laj", 5500000},
        {"MM7rqzepyqAMeZ4Vn7NBzAxsuR5SruLZZg", 4400000},
        {"MMQKQYrC4YA6EVn2DeKSvMiW5hgvBDTFQ6", 3300000},
        {"MQBNSnNdgQjTcNVoJSLvoAAj29pr1MUJdL", 2200000},
        {"MADNw1zEFwxdCKpvozyjQAG6yXZ4C582pN", 1100000},
        {"M8vjBXDnisgjvAZCj9jVFnq4sxEqbBDxZ7", 45600000},
        {"MBoLXcgSbmx1ubJgVYhDbEcqDdgprcjFN5", 45300000},
        {"MP6S7vJp6EtWvRhneeLxSmo8fJrbQaXvcS", 45200000}
    };

    auto stake_holders = fts_stake_holder::convert(stake_data);
    log::info(LOG_HEADER) << "test: seed: " << seed << ", stake size: " << stake_holders->size();

    if (false) {
        // select one item from stake_holders

        // first step
        fts_node::ptr tree = fts::build_merkle_tree(*stake_holders);
        fts_node::ptr selected = fts::select_by_fts(tree, seed);
        log::info(LOG_HEADER)
            << "tree one: " << encode_hash(tree->hash()) << ", stake: " << tree->stake();
        log::info(LOG_HEADER)
            << "selected one: " << encode_hash(selected->hash()) << ", stake: " << selected->stake();

        BITCOIN_ASSERT(selected->is_leaf());
        auto holder = selected->stake_holder();
        log::info(LOG_HEADER)
            << "holder one: " << holder->address() << ", stake: " << holder->stake();

        // second step
        fts_node::ptr tree2 = fts::build_merkle_tree(*stake_holders);
        fts_node::ptr selected2 = fts::select_by_fts(tree2, seed);
        log::info(LOG_HEADER)
            << "tree two: " << encode_hash(tree2->hash()) << ", stake: " << tree2->stake();
        log::info(LOG_HEADER)
            << "selected two: " << encode_hash(selected2->hash()) << ", stake: " << selected2->stake();

        BITCOIN_ASSERT(selected2->is_leaf());
        auto holder2 = selected2->stake_holder();
        log::info(LOG_HEADER)
            << "holder two: " << holder2->address() << ", stake: " << holder2->stake();

        // verify
        BITCOIN_ASSERT(tree->hash() == tree2->hash());
        BITCOIN_ASSERT(selected->hash() == selected2->hash());
    }

    {
        // select N items from stake_holders
        std::srand(std::time(nullptr));
        const size_t N = 1 + std::rand() % (stake_holders->size() * 2);

        // first step
        auto items = fts::select_by_fts(*stake_holders, seed, N);
        BITCOIN_ASSERT(items != nullptr);

        // second step
        auto check_items = fts::select_by_fts(*stake_holders, seed, N);
        BITCOIN_ASSERT(check_items != nullptr);

        // verify
        log::info(LOG_HEADER) << "choose " << items->size() << " items, seed: "
            << seed << ", stake size: " << stake_holders->size()
            << ", N: " << N;
        BITCOIN_ASSERT(items->size() == check_items->size());
        for (size_t i = 0; i < items->size(); ++i) {
            BITCOIN_ASSERT((*items)[i] == (*check_items)[i]);
            log::info(LOG_HEADER) << "     item " << i << ": " << (*items)[i]->to_string();
        }
    }
}


}
}
