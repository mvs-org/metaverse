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
#include <metaverse/bitcoin/utility/string.hpp>
#include <unordered_map>

namespace libbitcoin {
namespace chain {

const char* LOG_HEADER{"attenuation_model"};

// ASSET_TODO
class attenuation_model::impl
{
public:
    impl(const std::string& param)
        : model_param_(param)
    {
        parse_param();
    }

    const std::string& get_model_param() const {
        return model_param_;
    }

    // TYPE model type
    model_type get_model_type() const {
        return from_index(getnumber<uint8_t>("TYPE"));
    }

    // PN  current period number
    uint64_t get_period_number() const {
        return getnumber("PN");
    }

    // IQ  total issued quantity
    uint64_t get_issued_quantity() const {
        return getnumber("IQ");
    }

    // LQ  total locked quantity
    uint64_t get_locked_quantity() const {
        return getnumber("LQ");
    }

    // LP  total locked period
    uint64_t get_locked_period() const {
        return getnumber("LP");
    }

    // UCt size()==1 means fixed cycle
    std::vector<uint64_t> get_unlock_cycles() const {
        return get_numbers("UC");
    }

    // IRt size()==1 means fixed rate
    std::vector<uint8_t> get_issue_rates() const {
        return get_numbers<uint8_t>("IR");
    }

    // UQt size()==1 means fixed quantity
    std::vector<uint64_t> get_unlocked_quantities() const {
        return get_numbers("UQ");
    }

private:
    void parse_param() {
        auto kv_vec = bc::split(model_param_, ";");
        for (const auto& kv : kv_vec) {
            auto vec = bc::split(kv, "=");
            if (vec.size() == 2) {
                map_[vec[0]] = vec[1];
            } else {
                set_wrong_format();
                log::info(LOG_HEADER) << "key-value format is wrong";
                break;
            }
        }
    }

    template<typename T = uint64_t>
    T getnumber(const char* key) const {
        if (is_wrong_format()) {
            return 0;
        }
        auto iter = map_.find(key);
        if (iter != map_.end()) {
            try {
                auto num = std::stoull(iter->second);
                return num;
            } catch (...) {
                set_wrong_format();
                log::info(LOG_HEADER) << "caught exception in getnumber()";
            }
        }
        return 0;
    }

    template<typename T = uint64_t>
    std::vector<T> get_numbers(const char* key) const {
        std::vector<T> ret_vec;
        if (is_wrong_format()) {
            return ret_vec;
        }
        auto iter = map_.find(key);
        if (iter != map_.end()) {
            try {
                auto num_vec = bc::split(iter->second, ",");
                for (const auto& item : num_vec) {
                    auto num = std::stoull(item);
                    ret_vec.emplace_back(num);
                }
            } catch (...) {
                set_wrong_format();
                ret_vec.clear();
                log::info(LOG_HEADER) << "caught exception in getnumbers()";
            }
        }
        return ret_vec;
    }

    bool is_wrong_format() const {
        return is_wrong_foramt;
    }
    void set_wrong_format() const {
        const_cast<attenuation_model::impl*>(this)->is_wrong_foramt = true;
    }

private:
    // semicolon separates outer key-value entries.
    // comma separates inner container items of value.
    // empty value or non-exist entry means the key is unset.
    // example of fixed quantity model param:
    // "PN=0;TYPE=1;IQ=10000;LQ=9000;LP=60000;UC=20000;IR=0;UQ=3000"
    std::string model_param_;
    std::unordered_map<std::string, std::string> map_;
    bool is_wrong_foramt{false};
};

attenuation_model::attenuation_model(std::string&& param)
    : pimpl(std::make_unique<impl>(param))
{
}

attenuation_model::model_type attenuation_model::get_model_type() const
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
    return to_index(model_type::unused1);
}

uint8_t attenuation_model::to_index(attenuation_model::model_type model)
{
    return static_cast<typename std::underlying_type<model_type>::type>(model);
}

attenuation_model::model_type attenuation_model::from_index(uint32_t index)
{
    BITCOIN_ASSERT(check_model_index(index));
    return (model_type)index;
}

bool attenuation_model::check_model_index(uint32_t index)
{
    return index < get_first_unused_index();
}

bool attenuation_model::check_model_param(uint32_t index, const data_chunk& param)
{
    const model_type model = from_index(index);

    if (model == model_type::none) {
        return true;
    }

    else if (model == model_type::fixed_quantity) {
        // ASSET_TODO
        return true;
    }

    else if (model == model_type::fixed_rate) {
        // ASSET_TODO
        return true;
    }
    else {
        log::info(LOG_HEADER) << "Unsupported attenuation model: " << index;
        return false;
    }

    return false;
}

} // namspace chain
} // namspace libbitcoin
