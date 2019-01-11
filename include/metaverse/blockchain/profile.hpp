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
#ifndef MVS_BLOCKCHAIN_PROFILE_HPP
#define MVS_BLOCKCHAIN_PROFILE_HPP

#include <metaverse/blockchain/define.hpp>

namespace libbitcoin {
namespace blockchain {

enum class profile_type : uint32_t
{
    none,
    witness,
};

struct BCB_API profile_context
{
    profile_type type;
    const block_chain_impl& block_chain;
    std::pair<uint64_t, uint64_t> height_range;
    std::set<std::string> hex_public_keys;
};

class BCB_API profile
{
public:
    using ptr = std::shared_ptr<profile>;
    using list = std::vector<profile::ptr>;

    profile() = default;
    virtual ~profile() {}
    virtual profile_type get_type() const = 0;
    virtual profile::ptr get_profile(const profile_context&) = 0;

    static bool check_context(const profile_context&);
};

class BCB_API witness_profile final : public profile
{
public:
    using ptr = std::shared_ptr<witness_profile>;
    using list = std::vector<witness_profile::ptr>;

    witness_profile();
    ~witness_profile();

    profile_type get_type() const override { return profile_type::witness; }
    profile::ptr get_profile(const profile_context&) override;

    struct epoch_stat {
        uint64_t epoch_start_height;
        uint32_t witness_count;
        uint32_t total_dpos_block_count;
    } witness_epoch_stat;

    struct mining_stat {
        uint32_t witness_slot_num;
        uint32_t mined_block_count;
        uint32_t missed_block_count;
    };

    std::map<std::string, mining_stat> witness_mining_stat_map;

    bool operator== (const witness_profile& other) const;

    // serialization
    uint64_t serialized_size() const;
    bool from_data(reader& source);
    data_chunk to_data() const;
};

} // namespace blockchain
} // namespace libbitcoin

#endif
