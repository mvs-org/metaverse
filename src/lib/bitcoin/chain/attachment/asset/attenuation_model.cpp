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
    impl(model_index model, const std::string& param)
        : model_type_(model)
        , model_param_(param)
    {
    }

    model_index get_model_type() const {
        return model_type_;
    }

    const std::string& get_model_param() const {
        return model_param_;
    }

    // PN  current period number
    uint64_t get_period_number() const {
        return 0;
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
    model_index model_type_{model_index::none};
    // semicolon separates outer key-value entries.
    // comma separates inner container items of value.
    // empty value or non-exist entry means the key is unset.
    // example of fixed quantity model param:
    // "PN=0;IQ=10000;LQ=9000;LP=60000;UC=20000;IR=0;UQ=3000"
    std::string model_param_;
};

attenuation_model::attenuation_model(uint8_t index, const std::string& param)
    : pimpl(new impl(from_index(index), param))
{
}

attenuation_model::model_index attenuation_model::get_model_type() const
{
    return pimpl->get_model_type();
}

const std::string& attenuation_model::get_model_param() const
{
    return pimpl->get_model_param();
}

uint64_t attenuation_model::get_period_number() const
{
    return pimpl->get_period_number();
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

uint8_t attenuation_model::get_first_unused_index()
{
    return to_index(model_index::unused1);
}

uint8_t attenuation_model::to_index(attenuation_model::model_index model)
{
    return static_cast<typename std::underlying_type<model_index>::type>(model);
}

attenuation_model::model_index attenuation_model::from_index(uint32_t index)
{
    BITCOIN_ASSERT(check_model_index(index));
    return (model_index)index;
}

bool attenuation_model::check_model_index(uint32_t index)
{
    return index < get_first_unused_index();
}

bool attenuation_model::check_model_param(uint32_t index, const data_chunk& param)
{
    const model_index model = from_index(index);

    if (model == model_index::none) {
        return true;
    }

    else if (model == model_index::fixed_quantity) {
        // ASSET_TODO
        return true;
    }

    else if (model == model_index::fixed_rate) {
        // ASSET_TODO
        return true;
    }
    else {
        log::info("attenuation_model")
            << "Unsupported attenuation model: " << index;
        return false;
    }

    return false;
}

} // namspace chain
} // namspace libbitcoin
