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

constexpr size_t hex_public_key_size = 2 * ec_compressed_size;

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
    auto& hex_public_keys = context.hex_public_keys;

    if (!witness::is_begin_of_epoch(range.first)) {
        return nullptr;
    }
    if (!witness::is_in_same_epoch(range.first, range.second-1)) {
        return nullptr;
    }

    const auto epoch_start_height = range.first;

    epoch_stat res_epoch_stat = {};
    res_epoch_stat.epoch_start_height = epoch_start_height; // epoch_start_height

    auto sp_witnesses = witness::get().get_block_witnesses(epoch_start_height);
    if (!sp_witnesses) {
        return nullptr;
    }
    uint32_t witness_count = sp_witnesses->size();
    res_epoch_stat.witness_count = witness_count; // witness_count

    auto get_slot_num = [sp_witnesses](const std::string& hex_public_key) -> uint32_t {
        const auto pos = std::find_if(std::begin(*sp_witnesses), std::end(*sp_witnesses),
            [&hex_public_key](const witness::witness_id& item) {
                return item == to_chunk(hex_public_key);
            });
        if (pos == sp_witnesses->end()) {
            return max_uint32;
        }
        return std::distance(sp_witnesses->begin(), pos);
    };

    auto get_next_slot = [&witness_count](uint32_t curr_slot) -> uint32_t {
        return (curr_slot + 1) % witness_count;
    };

    std::vector<mining_stat*> mining_stat_vec(witness_count, nullptr);

    if (hex_public_keys.empty()) {
        for (auto& pubkey : hex_public_keys) {
            auto witness_slot_num = get_slot_num(pubkey);
            auto* stat_ptr = &witness_mining_stat_map[pubkey];
            stat_ptr->witness_slot_num = witness_slot_num; // witness_slot_num
            if (witness_slot_num < witness_count) {
                mining_stat_vec[witness_slot_num] = stat_ptr;
            }
        }
    }
    else {
        // if no public key is specified, then get all witnesses' profile
        for (size_t witness_slot_num = 0; witness_slot_num < witness_count; ++witness_slot_num) {
            auto pubkey = witness::witness_to_string((*sp_witnesses)[witness_slot_num]);
            auto* stat_ptr = &witness_mining_stat_map[pubkey];
            stat_ptr->witness_slot_num = witness_slot_num; // witness_slot_num
            mining_stat_vec[witness_slot_num] = stat_ptr;
        }
    }

    uint32_t mined_block_count = 0;
    uint32_t missed_block_count = 0;
    uint32_t total_dpos_block_count = 0;
    chain::header header;

    uint32_t prev_slot_num = max_uint32;
    uint32_t curr_slot_num = 0;
    for (auto height = range.first; height < range.second; ++height) {
        if (!chain.get_header(header, height)) {
            return nullptr;
        }
        if (!header.is_proof_of_dpos()) {
            continue;
        }
        ++total_dpos_block_count;

        curr_slot_num = static_cast<uint32_t>(header.nonce);
        if (mining_stat_vec[curr_slot_num] != nullptr) {
            ++mining_stat_vec[curr_slot_num]->mined_block_count; // mined_block_count
        }

        if (prev_slot_num != max_uint32) {
            for (auto next = get_next_slot(prev_slot_num);
                    next != curr_slot_num; next = get_next_slot(next)) {
                if (mining_stat_vec[next] != nullptr) {
                    ++mining_stat_vec[next]->missed_block_count; // missed_block_count
                }
            }
        }
        prev_slot_num = curr_slot_num;
    }

    res_epoch_stat.total_dpos_block_count = total_dpos_block_count; // total_dpos_block_count
    witness_epoch_stat = res_epoch_stat;

    return std::make_shared<witness_profile>(*this);
}

uint64_t witness_profile::serialized_size() const
{
    constexpr uint64_t epoch_stat_size = 8 + 4 + 4;
    constexpr uint64_t mining_stat_size = 4 + 4 + 4;
    constexpr uint64_t space_to_store_witness_count = 4;
    uint64_t stat_map_count = witness_mining_stat_map.size();
    uint64_t stat_map_item_size = hex_public_key_size + mining_stat_size;
    uint64_t stat_map_total_size = space_to_store_witness_count + (stat_map_count * stat_map_item_size);
    return epoch_stat_size + stat_map_total_size;

}

bool witness_profile::from_data(reader& source)
{
    epoch_stat e_stat;
    std::map<std::string, mining_stat> stat_map;

    e_stat.epoch_start_height = source.read_8_bytes_little_endian();
    e_stat.witness_count = source.read_4_bytes_little_endian();
    e_stat.total_dpos_block_count = source.read_4_bytes_little_endian();

    uint32_t witness_count = source.read_4_bytes_little_endian();

    for (uint32_t i = 0; i < witness_count; ++i) {
        auto pubkey = source.read_fixed_string(hex_public_key_size);
        mining_stat stat = {};
        stat.witness_slot_num = source.read_4_bytes_little_endian();
        stat.mined_block_count = source.read_4_bytes_little_endian();
        stat.missed_block_count = source.read_4_bytes_little_endian();
        stat_map[pubkey] = stat;
    }

    auto result = static_cast<bool>(source);
    if (!result) {
        return false;
    }

    witness_epoch_stat = e_stat;
    witness_mining_stat_map.swap(stat_map);

    return true;
}

data_chunk witness_profile::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    ostream_writer sink(ostream);

    sink.write_8_bytes_little_endian(witness_epoch_stat.epoch_start_height);
    sink.write_4_bytes_little_endian(witness_epoch_stat.witness_count);
    sink.write_4_bytes_little_endian(witness_epoch_stat.total_dpos_block_count);

    uint32_t witness_count = witness_mining_stat_map.size();
    sink.write_4_bytes_little_endian(witness_count);

    for (auto& pair : witness_mining_stat_map) {
        auto& hex_pubkey = pair.first;
        auto& stat = pair.second;
        sink.write_fixed_string(hex_pubkey, hex_public_key_size);
        sink.write_4_bytes_little_endian(stat.witness_slot_num);
        sink.write_4_bytes_little_endian(stat.mined_block_count);
        sink.write_4_bytes_little_endian(stat.missed_block_count);
    }

    ostream.flush();
    return data;
}

bool witness_profile::operator== (const witness_profile& other) const
{
    return to_data() == other.to_data();
}

} // namespace blockchain
} // namespace libbitcoin
