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
#include <metaverse/blockchain/profile.hpp>
#include <metaverse/blockchain/block_chain_impl.hpp>
#include <metaverse/consensus/witness.hpp>

namespace libbitcoin {
namespace blockchain {

bool profile::check_context(const profile_context& context)
{
    // check type
    auto& type = context.type;
    switch (type) {
        case profile_type::witness:
            break;
        case profile_type::none:
        default:
            return false;
    }
    // check height range
    auto& range = context.height_range;
    if (range.first >= range.second) {
        return false;
    }
    return true;
}

witness_profile::witness_profile()
    : profile()
{
}

witness_profile::~witness_profile()
{
}

profile::ptr witness_profile::get_profile(const profile_context& context)
{
    using witness = consensus::witness;

    if (!check_context(context)) {
        return nullptr;
    }

    auto& range = context.height_range;
    auto& chain = context.block_chain;
    auto& hex_public_key = context.hex_public_key;

    if (!witness::is_begin_of_epoch(range.first)) {
        return nullptr;
    }
    if (!witness::is_in_same_epoch(range.first, range.second-1)) {
        return nullptr;
    }

    const auto epoch_height = range.first;

    mining_stat stat = {};
    stat.epoch_start_height = epoch_height; // epoch_start_height

    auto sp_witnesses = witness::get().get_block_witnesses(epoch_height);
    if (!sp_witnesses) {
        return nullptr;
    }
    stat.witness_count = sp_witnesses->size(); // witness_count

    uint32_t mined_block_count = 0;
    for (auto height = range.first; height < range.second; ++height) {
        ec_compressed public_key;
        if (!chain.get_public_key(public_key, height)) {
            return nullptr;
        }
        if (encode_base16(public_key) != hex_public_key) {
            continue;
        }
        ++mined_block_count;
    }
    stat.mined_block_count = mined_block_count; // mined_block_coun

    const auto pos = std::find_if(std::begin(*sp_witnesses), std::end(*sp_witnesses),
        [&hex_public_key](const witness::witness_id& item) {
            return item == to_chunk(hex_public_key);
        });
    if (pos == sp_witnesses->end()) {
        return nullptr;
    }
    stat.witness_slot_num = std::distance(sp_witnesses->begin(), pos); // witness_slot_num

    witness_mining_stat = stat;

    return std::make_shared<witness_profile>(*this);
}

} // namespace blockchain
} // namespace libbitcoin
