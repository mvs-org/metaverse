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
#include <metaverse/blockchain/block_chain_impl.hpp>
#include <unordered_map>

namespace libbitcoin {
namespace chain {

namespace {
    const char* LOG_HEADER{"attenuation_model"};

    bool is_positive_number(uint64_t num) {
        return num > 0;
    };

    template<typename T>
    uint64_t sum_and_check_numbers(const std::vector<T>& container,
            std::function<bool(uint64_t)> predicate) {
        uint64_t sum = 0;
        for (const auto& num : container) {
            if (!predicate(num)) {
                return 0;
            }
            sum += num;
        }
        return sum;
    }

} // end of anonymous namespace

class attenuation_model::impl
{
public:
    impl(const std::string& param)
        : model_param_(param)
    {
        if (!parse_param()) {
            map_.clear();
        }
    }

    const std::string& get_model_param() const {
        return model_param_;
    }

    // TYPE model type
    model_type get_model_type() const {
        return from_index(getnumber<uint8_t>("TYPE"));
    }

    // PN  current period number
    uint64_t get_current_period_number() const {
        return getnumber("PN");
    }

    // LH  latest lock height
    uint64_t get_latest_lock_height() const {
        return getnumber("LH");
    }

    // LQ  total locked quantity
    uint64_t get_locked_quantity() const {
        return getnumber("LQ");
    }

    // LP  total locked period
    uint64_t get_locked_period() const {
        return getnumber("LP");
    }

    // UN  total unlock numbers
    uint64_t get_unlock_number() const {
        return getnumber("UN");
    }

    // UCt size()==1 means fixed cycle
    const std::vector<uint64_t>& get_unlock_cycles() const {
        return get_numbers("UC");
    }

    // IRt size()==1 means fixed rate
    const std::vector<uint64_t>& get_issue_rates() const {
        return get_numbers("IR");
    }

    // UQt size()==1 means fixed quantity
    const std::vector<uint64_t>& get_unlocked_quantities() const {
        return get_numbers("UQ");
    }

private:
    bool parse_param() {
        auto is_illegal_char = [](auto c){ return ! (std::isalnum(c) || (c == ',') || (c == ';') || (c == '=')); };
        auto iter = std::find_if(model_param_.begin(), model_param_.end(), is_illegal_char);
        if (iter != model_param_.end()) {
            log::info(LOG_HEADER) << "illegal char found at pos "
                << std::distance(model_param_.begin(), iter) << " : " << *iter;
            return false;
        }

        if (model_param_.find(",,") != std::string::npos) {
            log::info(LOG_HEADER) << "',,' is not allowed.";
            return false;
        }

        auto kv_vec = bc::split(model_param_, ";");
        for (const auto& kv : kv_vec) {
            auto vec = bc::split(kv, "=");
            if (vec.size() == 2) {
                if (vec[0].empty()) {
                    log::info(LOG_HEADER) << "key-value format is wrong, key is empty in " << kv;
                    return false;
                }
                if (vec[1].empty()) {
                    continue; // empty value as unset.
                }
                try {
                    std::vector<uint64_t> num_vec;
                    auto str_vec = bc::split(vec[1], ",");
                    for (const auto& item : str_vec) {
                        auto num = std::stoull(item);
                        num_vec.emplace_back(num);
                    }
                    map_[vec[0]] = std::move(num_vec);
                } catch (const std::exception& e) {
                    log::info(LOG_HEADER) << "exception caught: " << e.what();
                    return false;
                }
            } else {
                log::info(LOG_HEADER) << "key-value format is wrong, should be key=value format.";
                return false;
            }
        }
        return true;
    }

    template<typename T = uint64_t>
    T getnumber(const char* key) const {
        auto iter = map_.find(key);
        if (iter == map_.end()) {
            return 0;
        }
        return iter->second[0];
    }

    const std::vector<uint64_t>& get_numbers(const char* key) const {
        auto iter = map_.find(key);
        if (iter == map_.end()) {
            return empty_num_vec;
        }
        return iter->second;
    }

private:
    // semicolon separates outer key-value entries.
    // comma separates inner container items of value.
    // empty value or non-exist entry means the key is unset.
    // * example of fixed quantity model param:
    // "PN=0;LH=20000;TYPE=1;LQ=9000;LP=60000;UN=3"
    // * example of fixed rate model param:
    // "PN=0;LH=20000;TYPE=2;LQ=9000;LP=60000;UN=3,IR=8;UC=...;UQ=..."
    // * example of custom model param:
    // "PN=0;LH=20000;TYPE=3;LQ=9000;LP=60000;UN=3;UC=20000,20000,20000;UQ=3000,3000,3000"
    std::string model_param_;

    // auxilary data
    std::unordered_map<std::string, std::vector<uint64_t>> map_;
    static const std::vector<uint64_t> empty_num_vec;
};

const std::vector<uint64_t> attenuation_model::impl::empty_num_vec;

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

uint64_t attenuation_model::get_current_period_number() const
{
    return pimpl->get_current_period_number();
}

uint64_t attenuation_model::get_latest_lock_height() const
{
    return pimpl->get_latest_lock_height();
}

uint64_t attenuation_model::get_locked_quantity() const
{
    return pimpl->get_locked_quantity();
}

uint64_t attenuation_model::get_locked_period() const
{
    return pimpl->get_locked_period();
}

uint64_t attenuation_model::get_unlock_number() const
{
    return pimpl->get_unlock_number();
}

const std::vector<uint64_t>& attenuation_model::get_unlock_cycles() const
{
    return pimpl->get_unlock_cycles();
}

const std::vector<uint64_t>& attenuation_model::get_issue_rates() const
{
    return pimpl->get_issue_rates();
}

const std::vector<uint64_t>& attenuation_model::get_unlocked_quantities() const
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

bool attenuation_model::check_model_param(const data_chunk& param)
{
    attenuation_model parser(std::string(param.begin(), param.end()));

    const auto model = parser.get_model_type();

    // model_type::none is equivalent to
    // the scrpit pattern is not pay_key_hash_with_attenuation_model
    if (model == model_type::none) {
        log::info(LOG_HEADER) << "model_type::none should not has pay_key_hash_with_attenuation_model script pattern.";
        return false;
    }

    auto&& LQ = parser.get_locked_quantity();
    auto&& LP = parser.get_locked_period();
    auto&& UN = parser.get_unlock_number();

    // common condition : LQ > 0
    if (!is_positive_number(LQ)) {
        log::info(LOG_HEADER) << "common param error: LQ <= 0";
        return false;
    }
    // common condition : LP > 0
    if (!is_positive_number(LP)) {
        log::info(LOG_HEADER) << "common param error: LP <= 0";
        return false;
    }
    // common condition : UN > 0
    if (!is_positive_number(UN)) {
        log::info(LOG_HEADER) << "common param error: UN <= 0";
        return false;
    }

    if (model == model_type::fixed_quantity) {
        // given LQ, LP, UN, then
        // LP >= UN and LQ >= UN
        if (LP < UN) {
            log::info(LOG_HEADER) << "fixed_quantity param error: LP < UN";
            return false;
        }
        if (LQ < UN) {
            log::info(LOG_HEADER) << "fixed_quantity param error: LQ < UN";
            return false;
        }
        return true;
    }

    // Why convert to custom model is needed?
    // Because the computing of float's exponential
    // or other complex expression is 'inaccurate',
    // you may get different results of multiple computing.
    auto is_convert_to_custom = false;

    if (model == model_type::fixed_rate) {
        // given LQ, LP, UN, IR, UC, UQ,
        // then IR.size == 1 and IR > 0
        // and satisfy custom param conditions.
        is_convert_to_custom = true;
        auto&& IR = parser.get_issue_rates();
        if (IR.size() != 1) {
            log::info(LOG_HEADER) << "fixed_rate param error: IR.size() != 1";
            return false;
        }
        if (!is_positive_number(IR[0])) {
            log::info(LOG_HEADER) << "fixed_rate param error: IR <= 0";
            return false;
        }
    }

    if ((model == model_type::custom) || is_convert_to_custom) {
        // given LQ, LP, UN, UC, UQ, then
        // LQ = sum(UQ) and all UQ > 0 and UQ.size == UN
        // LP = sum(UC) and all UC > 0 and UC.size == UN
        auto&& UC = parser.get_unlock_cycles();
        auto&& UQ = parser.get_unlocked_quantities();
        if (UC.size() != UN) {
            log::info(LOG_HEADER) << "custom param error: UC.size() != UN";
            return false;
        }
        if (UQ.size() != UN) {
            log::info(LOG_HEADER) << "custom param error: UQ.size() != UN";
            return false;
        }
        if (sum_and_check_numbers(UC, is_positive_number) != LP) {
            log::info(LOG_HEADER) << "custom param error: LP != sum(UC) or exist UC <= 0";
            return false;
        }
        if (sum_and_check_numbers(UQ, is_positive_number) != LQ) {
            log::info(LOG_HEADER) << "custom param error: LQ != sum(UQ) or exist UQ <= 0";
            return false;
        }
        return true;
    }

    log::info(LOG_HEADER) << "Unsupported attenuation model: " << std::to_string(to_index(model));
    return false;
}

uint64_t attenuation_model::get_available_asset_amount(
        uint64_t asset_amount, uint64_t diff_height, const data_chunk& model_param)
{
    // ASSET_TODO
    return 0;
}

} // namspace chain
} // namspace libbitcoin
