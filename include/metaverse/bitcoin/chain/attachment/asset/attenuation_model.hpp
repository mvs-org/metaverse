/**
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of mvs-node.
 *
 * metaverse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
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
#ifndef MVS_CHAIN_ATTACHMENT_ASSET_ATTENUATION_MODEL_HPP
#define MVS_CHAIN_ATTACHMENT_ASSET_ATTENUATION_MODEL_HPP

#include <cstdint>
#include <metaverse/bitcoin/chain/point.hpp>
#include <metaverse/bitcoin/chain/script/script.hpp>
#include <metaverse/bitcoin/define.hpp>

using namespace libbitcoin::chain;

// forward declaration
namespace libbitcoin {
namespace blockchain {
    class block_chain_impl;
}
}

namespace libbitcoin {
namespace chain {

class attenuation_model
{
public:
    enum class model_type : uint8_t
    {
        none = 0,
        fixed_quantity = 1,
        custom = 2,
        unused1 = 3,
        unused2 = 4,
        unused3 = 5,
        unused4 = 6,
        unused5 = 7,
        invalid = 8
    };

    attenuation_model(std::string&& param);

    static uint8_t get_first_unused_index();
    static uint8_t to_index(model_type model);
    static model_type from_index(uint32_t index);

    static bool check_model_index(uint32_t index);
    static bool check_model_param(const data_chunk& param);
    static bool check_model_param(const transaction& tx, const blockchain::block_chain_impl& chain);
    static bool check_model_param_format(const data_chunk& param);
    static bool check_model_param_initial(const data_chunk& param, uint64_t total_amount);
    static bool check_model_param_initial(std::string& param, uint64_t total_amount);
    static bool check_model_param_immutable(const data_chunk& previous, const data_chunk& current);
    static uint64_t get_available_asset_amount(uint64_t asset_amount, uint64_t diff_height,
            const data_chunk& model_param, std::shared_ptr<data_chunk> new_param_ptr = nullptr);
    static uint64_t get_diff_height(const data_chunk& prev_param, const data_chunk& param);

    const std::string& get_model_param() const;
    data_chunk get_new_model_param(uint64_t PN, uint64_t LH) const;

    // mutable params of the model
    uint64_t get_current_period_number() const; // PN  current period number
    uint64_t get_latest_lock_height() const;    // LH  latest lock height

    // immutable params of the model
    model_type get_model_type() const;       // TYPE attenuation model type
    uint64_t get_locked_quantity() const;    // LQ  total locked quantity
    uint64_t get_locked_period() const;      // LP  total locked period
    uint64_t get_unlock_number() const;      // UN  total unlock numbers
    const std::vector<uint64_t>& get_unlock_cycles() const;        // UCt size()==1 means fixed cycle
    const std::vector<uint64_t>& get_unlocked_quantities() const;  // UQt size()==1 means fixed quantity

private:
    class impl;
    std::unique_ptr<impl> pimpl;
};

} // namespace chain
} // namespace libbitcoin

#endif

