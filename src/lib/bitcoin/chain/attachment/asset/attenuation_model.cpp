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

#include <metaverse/bitcoin/chain/attachment/asset/attenuation_model.hpp>

namespace libbitcoin {
namespace chain {

// ASSET_TODO
class attenuation_model::impl
{
public:
    impl(uint8_t index, const std::string& param)
        : model_index_((model_index)index)
        , model_param_(param)
    {
    }

    model_index get_model_index() const {
        return model_index_;
    }

    const std::string& get_model_param() const {
        return model_param_;
    }

    // IQ  total issued quantity
    uint64_t get_issued_quantity() const {
        return 0;
    }

    // LQ  total locked quantity
    uint64_t get_locked_quantity() const {
        return 0;
    }

    // LP  total locked period
    uint64_t get_locked_period() const {
        return 0;
    }

    // UCt size()==1 means fixed cycle
    std::vector<uint64_t> get_unlock_cycles() const {
        std::vector<uint64_t> cycles;
        return cycles;
    }

    // IRt size()==1 means fixed rate
    std::vector<uint8_t> get_issue_rates() const {
        std::vector<uint8_t> rates;
        return rates;
    }

    // UQt size()==1 means fixed quantity
    std::vector<uint64_t> get_unlocked_quantities() const {
        std::vector<uint64_t> quantities;
        return quantities;
    }

private:
    model_index model_index_{model_index::none};
    std::string model_param_;
};

attenuation_model::attenuation_model(uint8_t index, const std::string& param)
    : pimpl(new impl(index, param))
{
}

attenuation_model::model_index attenuation_model::get_model_index() const
{
    return pimpl->get_model_index();
}

const std::string& attenuation_model::get_model_param() const
{
    return pimpl->get_model_param();
}

uint64_t attenuation_model::get_issued_quantity() const
{
    return pimpl->get_issued_quantity();
}

uint64_t attenuation_model::get_locked_quantity() const
{
    return pimpl->get_locked_quantity();
}

uint64_t attenuation_model::get_locked_period() const
{
    return pimpl->get_locked_period();
}

std::vector<uint64_t> attenuation_model::get_unlock_cycles() const
{
    return pimpl->get_unlock_cycles();
}

std::vector<uint8_t> attenuation_model::get_issue_rates() const
{
    return pimpl->get_issue_rates();
}

std::vector<uint64_t> attenuation_model::get_unlocked_quantities() const
{
    return pimpl->get_unlocked_quantities();
}

bool attenuation_model::check_model_index(uint32_t index)
{
    return index < ATTENUATION_MODEL_FIRST_UNUSED;
}

// ASSET_TODO
bool attenuation_model::check_model_param(uint32_t index, const data_chunk& param)
{
    return true;
}

} // namspace chain
} // namspace libbitcoin
