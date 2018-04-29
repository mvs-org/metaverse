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

    std::map<attenuation_model::model_type, std::vector<std::string>> model_keys_map {
        {attenuation_model::model_type::fixed_quantity, {"PN","LH","TYPE","LQ","LP","UN"}},
        {attenuation_model::model_type::fixed_rate,     {"PN","LH","TYPE","LQ","LP","UN","IR","UC","UQ"}},
        {attenuation_model::model_type::custom,         {"PN","LH","TYPE","LQ","LP","UN","UC","UQ"}}
    };

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

    template<typename ForwardIterator>
    ForwardIterator find_nth_element(
            ForwardIterator first,
            ForwardIterator last,
            size_t nth,
            uint8_t elem)
    {
        if (nth == 0) {
            return last;
        }
        auto iter = std::find(first, last, elem);
        while ((--nth > 0) && (iter != last)) {
            iter = std::find(++iter, last, elem);
        }
        return iter;
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

    data_chunk get_new_model_param(uint64_t PN, uint64_t LH) const {
        auto iter = find_nth_element(model_param_.begin(), model_param_.end(), 2, ';');
        if (iter == model_param_.end()) {
            return data_chunk();
        }
        auto prefix = "PN=" + std::to_string(PN) + ";LH=" + std::to_string(LH);
        auto suffix = model_param_.substr(iter - model_param_.begin());
        return to_chunk(prefix + suffix);
    }

private:
    bool check_keys() {
        auto model = get_model_type();
        if (model == model_type::none) {
            return true;
        }
        const auto& keys = model_keys_map[model];
        if (map_.size() != keys.size()) {
            log::info(LOG_HEADER) << "keys number for model type " << to_index(model)
                << " is not exactly to " << keys.size();
            return false;
        }
        for (size_t i = 0; i < keys.size(); ++i) {
            if (map_.find(keys[i]) == map_.end()) {
                log::info(LOG_HEADER) << "model type " << to_index(model) << "needs key " << keys[i] << " but missed.";
                return false;
            }
        }
        return true;
    }

    bool parse_param() {
        if (model_param_.empty()) {
            return true;
        }
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

        const auto& kv_vec = bc::split(model_param_, ";");
        if (kv_vec.size() < 6) {
            log::info(LOG_HEADER) << "model param is " << model_param_
                << ", the model param should at least contain keys of PN, LH, TYPE, LQ, LP, UN";
            return false;
        }
        if (kv_vec[0].find("PN=") != 0) {
            log::info(LOG_HEADER) << "the model param first key must be PN";
            return false;
        }
        if (kv_vec[1].find("LH=") != 0) {
            log::info(LOG_HEADER) << "the model param second key must be LH";
            return false;
        }

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
                    if (map_.find(vec[0]) != map_.end()) {
                        log::info(LOG_HEADER) << "key-value format is wrong, duplicate key : " << vec[0];
                        return false;
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

        // check keys after map is constructed
        if (!check_keys()) {
            log::info(LOG_HEADER) << "check keys of model param failed ";
            return false;
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

data_chunk attenuation_model::get_new_model_param(uint64_t PN, uint64_t LH) const
{
    return pimpl->get_new_model_param(PN, LH);
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
    if (!check_model_index(index)) {
        log::info(LOG_HEADER) << "model index is wrong: index = " << index;
        return model_type::none;
    }
    return (model_type)index;
}

bool attenuation_model::check_model_index(uint32_t index)
{
    return index < get_first_unused_index();
}

bool attenuation_model::check_model_param_immutable(const data_chunk& previous, const data_chunk& current)
{
    if (previous.empty() || current.empty()) {
        return false;
    }

    auto iter1 = find_nth_element(previous.begin(), previous.end(), 2, ';');
    auto iter2 = find_nth_element(current.begin(), current.end(), 2, ';');

    return std::equal(iter1, previous.end(), iter2);
}

bool attenuation_model::check_model_param(const data_chunk& param, bool initial)
{
    attenuation_model parser(std::string(param.begin(), param.end()));

    const auto model = parser.get_model_type();

    // model_type::none is equivalent to
    // the scrpit pattern is not pay_key_hash_with_attenuation_model
    if (model == model_type::none) {
        log::info(LOG_HEADER) << "model_type::none should not has pay_key_hash_with_attenuation_model script pattern.";
        return false;
    }

    auto PN = parser.get_current_period_number();
    auto LH = parser.get_latest_lock_height();
    auto LQ = parser.get_locked_quantity();
    auto LP = parser.get_locked_period();
    auto UN = parser.get_unlock_number();

    // common condition : initial PN == 0
    if (initial) {
        if (PN != 0) {
            log::info(LOG_HEADER) << "common param error: initial PN != 0";
            return false;
        }
    }

    // common condition : PN < UN
    if (PN >= UN) {
        log::info(LOG_HEADER) << "common param error: PN >= UN";
        return false;
    }
    // common condition : LH > 0
    if (!is_positive_number(LH)) {
        log::info(LOG_HEADER) << "common param error: LH <= 0";
        return false;
    }
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

    auto is_fixed_cycle = false;
    switch (model) {
        case model_type::fixed_quantity:
        case model_type::fixed_rate:
            is_fixed_cycle = true;
            break;
        default:
            break;
    }

    if (is_fixed_cycle) {
        auto UC = LP / UN;
        if (initial) { // initial cycle
            if (LH != UC) {
                log::info(LOG_HEADER) << "fixed cycle param error: initial LH != UC";
                return false;
            }
        } else if (PN + 1 == UN) { // last cycle
            if (PN * UC + LH > LP) {
                log::info(LOG_HEADER) << "fixed cycle param error: last cycle PN * UC + LH > LP";
                return false;
            }
        } else {
            if (LH > UC) {
                log::info(LOG_HEADER) << "fixed cycle param error: LH > UC";
                return false;
            }
        }
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
        const auto& IR = parser.get_issue_rates();
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
        const auto& UC = parser.get_unlock_cycles();
        const auto& UQ = parser.get_unlocked_quantities();
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
        if (initial) {
            if (LH != UC[0]) {
                log::info(LOG_HEADER) << "custom param error: initial LH != UC[0]";
                return false;
            }
        } else if (LH > UC[PN]) {
            log::info(LOG_HEADER) << "custom param error: LH > UC";
            return false;
        }
        return true;
    }

    log::info(LOG_HEADER) << "Unsupported attenuation model: " << std::to_string(to_index(model));
    return false;
}

uint64_t attenuation_model::get_available_asset_amount(
        uint64_t asset_amount, uint64_t diff_height,
        const data_chunk& param, std::shared_ptr<data_chunk> new_param_ptr)
{
    if (asset_amount == 0) {
        return 0;
    }

    attenuation_model parser(std::string(param.begin(), param.end()));

    const auto model = parser.get_model_type();

    // model_type::none is equivalent to
    // the scrpit pattern is not pay_key_hash_with_attenuation_model
    if (model == model_type::none) {
        log::info(LOG_HEADER) << "model_type::none should not has pay_key_hash_with_attenuation_model script pattern.";
        return asset_amount;
    }

    auto PN = parser.get_current_period_number();
    auto LH = parser.get_latest_lock_height();
    auto LQ = parser.get_locked_quantity();
    auto LP = parser.get_locked_period();
    auto UN = parser.get_unlock_number();

    auto available = (asset_amount > LQ) ? (asset_amount - LQ) : 0;

    if (diff_height < LH) { // no maturity, still all locked
        if (new_param_ptr) {
            // update PN, LH
            LH -= diff_height;
            *new_param_ptr = parser.get_new_model_param(PN, LH);
        }
        return available;
    }

    if (model == model_type::fixed_quantity) {
        auto UC = LP / UN;
        auto locked_height = (diff_height - LH) + ((PN + 1) * UC);
        if (locked_height >= LP) { // include the last unlock cycle, release all
            return asset_amount;
        }
        auto new_cycles = std::min((locked_height / UC - PN), (LP - PN - 1));
        if (new_param_ptr) {
            // update PN, LH
            PN = LP + new_cycles;
            if ((PN + 1) == LP) {
                LH = LP - locked_height;
            } else {
                LH = (PN + 1) * UC - locked_height;
            }
            *new_param_ptr = parser.get_new_model_param(PN, LH);
        }
        auto UQ = LQ / UN;
        available += new_cycles * UQ;
        return available;
    }

    auto is_convert_to_custom = false;
    switch (model) {
        case model_type::fixed_rate:
            is_convert_to_custom = true;
            break;
        default:
            break;
    }

    if ((model == model_type::custom) || is_convert_to_custom) {
        const auto& UC = parser.get_unlock_cycles();
        const auto& UQ = parser.get_unlocked_quantities();
        available += UQ[PN];
        diff_height -= LH;
        ++PN;
        while ((PN < UN) && (diff_height >= UC[PN])) {
            available += UQ[PN];
            diff_height -= UC[PN];
            ++PN;
        }
        if (PN == UN) { // include the last unlock cycle, release all
            return asset_amount;
        }
        if (new_param_ptr) {
            // update PN, LH
            LH = UC[PN] - diff_height;
            *new_param_ptr = parser.get_new_model_param(PN, LH);
        }
        return available;
    }

    log::info(LOG_HEADER) << "Unsupported attenuation model: " << std::to_string(to_index(model));
    return asset_amount;
}

} // namspace chain
} // namspace libbitcoin
