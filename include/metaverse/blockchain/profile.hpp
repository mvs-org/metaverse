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
    std::string did;
    std::function<bool(const chain::output&)> filter;
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
    witness_profile();
    ~witness_profile();

    profile_type get_type() const override { return profile_type::witness; }
    profile::ptr get_profile(const profile_context&) override;

    struct mining_stat {
        uint32_t epoch_start_height;
        uint32_t witness_count;
        uint32_t witness_slot_num;
        uint32_t mined_block_count;
        ec_compressed public_key_data;
    } witness_mining_stat;
};

} // namespace blockchain
} // namespace libbitcoin

#endif
