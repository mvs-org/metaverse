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
#include <metaverse/blockchain/validate_transaction.hpp>
#include <unordered_map>
#include <memory>

namespace libbitcoin {
namespace chain {

static BC_CONSTEXPR uint64_t max_inflation_rate = 100000;
static BC_CONSTEXPR uint64_t max_unlock_number = 100;

namespace {
    const char* LOG_HEADER{"attenuation_model"};

    std::string chunk_to_string(const data_chunk& chunk) {
        return std::string(chunk.begin(), chunk.end());
    }

    std::vector<std::pair<std::string, std::string>> key_name_pairs{
        {"PN",      "current_period_nbr"},
        {"LH",      "next_interval"},
        {"TYPE",    "type"},
        {"LQ",      "lock_quantity"},
        {"LP",      "lock_period"},
        {"UN",      "total_period_nbr"},
        {"IR",      "inflation_rate"},
        {"UC",      "custom_lock_number_array"},
        {"UQ",      "custom_lock_quantity_array"}
    };

    std::map<attenuation_model::model_type, std::vector<std::string>> model_keys_map {
        {attenuation_model::model_type::fixed_quantity, {"PN","LH","TYPE","LQ","LP","UN"}},
        {attenuation_model::model_type::custom,         {"PN","LH","TYPE","LQ","LP","UN","UC","UQ"}},
        {attenuation_model::model_type::fixed_inflation,{"PN","LH","TYPE","LQ","LP","UN","IR","UC","UQ"}}
    };

    std::vector<std::string> inflation_model_initial_keys{"PN","LH","TYPE","LQ","LP","UN","IR"};

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

        typedef typename std::iterator_traits<ForwardIterator>::reference Tref;
        return std::find_if(first, last, [&](Tref x) {
                return (x == elem) && (--nth == 0);
            }
        );
    }


} // end of anonymous namespace

class attenuation_model::impl
{
public:
    impl(const std::string& param, bool is_init)
        : model_param_(param)
    {
        if (!parse_param(is_init)) {
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

    // IR  inflation rate
    uint64_t get_inflation_rate() const {
        return getnumber("IR");
    }

    // UCt size()==1 means fixed cycle
    const std::vector<uint64_t>& get_unlock_cycles() const {
        return get_numbers("UC");
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
    bool validate_keys(model_type model, const std::vector<std::string>& keys) {
        if (map_.size() != keys.size()) {
            log::debug(LOG_HEADER) << "The size of keys " << map_.size()
                << " for model type " << std::to_string(to_index(model))
                << " does not equal " << keys.size();
            return false;
        }

        for (size_t i = 0; i < keys.size(); ++i) {
            if (map_.find(keys[i]) == map_.end()) {
                log::debug(LOG_HEADER) << "model type " << std::to_string(to_index(model))
                    << " needs key " << keys[i] << " but missed.";
                return false;
            }
        }

        return true;
    }

    bool check_keys(bool is_init) {
        auto model = get_model_type();
        if (model == model_type::none) {
            return true;
        }

        if (is_init && model == model_type::fixed_inflation) {
            return validate_keys(model, inflation_model_initial_keys);
        }
        else {
            return validate_keys(model, model_keys_map[model]);
        }
    }

    bool parse_uint64(const std::string& param, uint64_t& value)
    {
        for (auto& i : param){
            if (!std::isalnum(i)) {
                return false;
            }
        }

        value = std::stoull(param);
        return true;
    }

    bool parse_param(bool is_init=false) {
        if (model_param_.empty()) {
            return true;
        }
        auto is_illegal_char = [](auto c){ return ! (std::isalnum(c) || (c == ',') || (c == ';') || (c == '=')); };
        auto iter = std::find_if(model_param_.begin(), model_param_.end(), is_illegal_char);
        if (iter != model_param_.end()) {
            log::debug(LOG_HEADER) << "illegal char found at pos "
                << std::distance(model_param_.begin(), iter) << " : " << *iter;
            return false;
        }

        if (model_param_.find(",,") != std::string::npos) {
            log::debug(LOG_HEADER) << "',,' is not allowed.";
            return false;
        }

        const auto& kv_vec = bc::split(model_param_, ";", true);
        if (kv_vec.size() < 6) {
            log::debug(LOG_HEADER) << "model param is " << model_param_
                << ", the model param should at least contain keys of PN, LH, TYPE, LQ, LP, UN";
            return false;
        }
        if (kv_vec[0].find("PN=") != 0) {
            log::debug(LOG_HEADER) << "the model param first key must be PN";
            return false;
        }
        if (kv_vec[1].find("LH=") != 0) {
            log::debug(LOG_HEADER) << "the model param second key must be LH";
            return false;
        }

        for (const auto& kv : kv_vec) {
            auto vec = bc::split(kv, "=", true);
            if (vec.size() == 2) {
                auto key = vec[0];
                auto values = vec[1];
                if (key.empty()) {
                    log::debug(LOG_HEADER) << "key-value format is wrong, key is empty in " << kv;
                    return false;
                }

                if (map_.find(key) != map_.end()) {
                    log::debug(LOG_HEADER) << "key-value format is wrong, duplicate key : " << key;
                    return false;
                }

                if (values.empty()) {
                    continue; // empty value as unset.
                }

                try {
                    std::vector<uint64_t> num_vec;
                    uint64_t num = 0;
                    if (attenuation_model::is_multi_value_key(key)) {
                        auto str_vec = bc::split(values, ",", true);
                        for (const auto& item : str_vec) {
                            if (parse_uint64(item, num)) {
                                num_vec.emplace_back(num);
                            }
                            else {
                                log::debug(LOG_HEADER) << "value is not a number: " << item;
                                return false;
                            }
                        }
                    }
                    else {
                        if (parse_uint64(values, num)) {
                            num_vec.emplace_back(num);
                        }
                        else {
                            log::debug(LOG_HEADER) << "value is not a number: " << values;
                            return false;
                        }
                    }

                    map_[key] = std::move(num_vec);
                }
                catch (const std::exception& e) {
                    log::debug(LOG_HEADER) << "exception caught: " << e.what();
                    return false;
                }
            } else {
                log::debug(LOG_HEADER) << "key-value format is wrong, should be key=value format. " << model_param_;
                return false;
            }
        }

        // check keys after map is constructed
        if (!check_keys(is_init)) {
            log::debug(LOG_HEADER) << "check keys of model param failed ";
            return false;
        }
        return true;
    }

    template<typename T = uint64_t>
    T getnumber(const std::string& key) const {
        BITCOIN_ASSERT(!attenuation_model::is_multi_value_key(key));
        auto iter = map_.find(key);
        if (iter == map_.end()) {
            return 0;
        }
        return iter->second[0];
    }

    const std::vector<uint64_t>& get_numbers(const std::string& key) const {
        BITCOIN_ASSERT(attenuation_model::is_multi_value_key(key));
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
    // PN, LH is mutable and generated by program.
    // * example of fixed quantity model param:
    // "PN=0;LH=20000;TYPE=1;LQ=9000;LP=60000;UN=3"
    // * example of custom model param:
    // "PN=0;LH=20000;TYPE=2;LQ=9000;LP=60000;UN=3;UC=20000,20000,20000;UQ=3000,3000,3000"
    // * example of fixed inflation reate model param
    // "PN=0;LH=1000;TYPE=3;LQ=20000000;LP=12000;UN=12;IR=8"
    std::string model_param_;

    // auxilary data
    std::unordered_map<std::string, std::vector<uint64_t>> map_;
    static const std::vector<uint64_t> empty_num_vec;
};

const std::vector<uint64_t> attenuation_model::impl::empty_num_vec;

attenuation_model::attenuation_model(const std::string& param, bool is_init)
    : pimpl(std::make_unique<impl>(param, is_init))
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

uint64_t attenuation_model::get_inflation_rate() const
{
    return pimpl->get_inflation_rate();
}

const std::vector<uint64_t>& attenuation_model::get_unlock_cycles() const
{
    return pimpl->get_unlock_cycles();
}

const std::vector<uint64_t>& attenuation_model::get_unlocked_quantities() const
{
    return pimpl->get_unlocked_quantities();
}

bool attenuation_model::is_multi_value_key(const std::string& key)
{
    return key == "UC" || key == "UQ";
}

std::string attenuation_model::get_name_of_key(const std::string& key)
{
    for (const auto& pair : key_name_pairs) {
        if (key == pair.first) {
            return pair.second;
        }
    }
    BITCOIN_ASSERT(false);
    return "";
}

std::string attenuation_model::get_key_of_name(const std::string& name)
{
    const std::string prefix = "model_";
    if (name.find(prefix) != 0) {
        return "";
    }
    auto compare_name = name.substr(prefix.size());
    for (const auto& pair : key_name_pairs) {
        if (compare_name == pair.second) {
            return pair.first;
        }
    }
    return "";
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
        log::debug(LOG_HEADER) << "model index is wrong: index = " << index;
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

bool attenuation_model::check_model_param_format(const data_chunk& param)
{
    attenuation_model parser(std::string(param.begin(), param.end()));

    const auto model = parser.get_model_type();

    // model_type::none is equivalent to
    // the scrpit pattern is not pay_key_hash_with_attenuation_model
    if (model == model_type::none) {
        if (!param.empty()) {
            log::debug(LOG_HEADER)
                << "check_model_param, wrong model param format : "
                << parser.get_model_param();
        }
        return false;
    }

    return true;
}

bool attenuation_model::check_model_param(const data_chunk& param, uint64_t total_amount)
{
    std::string model_param(param.begin(), param.end());
    return check_model_param_initial(model_param, total_amount, false);
}

bool attenuation_model::check_model_param_initial(std::string& param, uint64_t total_amount, bool is_init)
{
    bool has_prefix = param.find("PN=") == 0;

    attenuation_model parser(has_prefix ? param : ("PN=0;LH=0;" + param), is_init);

    const auto model = parser.get_model_type();

    // model_type::none is equivalent to
    // the scrpit pattern is not pay_key_hash_with_attenuation_model
    if (model == model_type::none) {
        if (!param.empty()) {
            log::debug(LOG_HEADER)
                << "check_model_param, wrong model param in intial : "
                << parser.get_model_param();
        }
        return false;
    }

    auto PN = parser.get_current_period_number();
    auto LH = parser.get_latest_lock_height();
    auto LQ = parser.get_locked_quantity();
    auto LP = parser.get_locked_period();
    auto UN = parser.get_unlock_number();
    const auto& UCs = parser.get_unlock_cycles();

    // common condition : initial PN = 0
    if (PN != 0) {
        log::debug(LOG_HEADER) << "common initial param error: PN != 0";
        return false;
    }

    // common condition : LQ <= IQ (utxo's asset amount)
    if (LQ > total_amount) {
        log::debug(LOG_HEADER) << "common initial param error: LQ > IQ";
        return false;
    }

    if (!check_model_param_common(parser)) {
        return false;
    }

    if (model == model_type::fixed_quantity || model == model_type::custom) {
        // add prefix of PN,LH, check after ensured UN > 0
        auto initial_lock_height = (!UCs.empty()) ? UCs[0] : (LP / UN);
        if (!has_prefix) {
            LH = initial_lock_height;
            if (is_init) {
                param = "PN=0;LH=" + std::to_string(LH) + ";" + param;
            }
        }
        else if (LH != initial_lock_height) {
            log::debug(LOG_HEADER) << "common initial param error: LH != " << initial_lock_height;
            return false;
        }
    }

    if (model == model_type::fixed_quantity) {
        return check_model_param_un(parser);
    }
    else if (model == model_type::fixed_inflation) {
        return check_model_param_initial_fixed_inflation(param, total_amount, parser, is_init);
    }
    else if (model == model_type::custom) {
        return check_model_param_uc_uq(parser);
    }

    log::debug(LOG_HEADER) << "check_model_param_initial, Unsupported attenuation model: "
        << std::to_string(to_index(model));
    return false;
}

bool attenuation_model::check_model_param_common(attenuation_model& parser)
{
    auto LQ = parser.get_locked_quantity();
    auto LP = parser.get_locked_period();
    auto UN = parser.get_unlock_number();
    auto PN = parser.get_current_period_number();

    // common condition : PN < UN
    if (PN >= UN) {
        log::debug(LOG_HEADER) << "common param error: PN >= UN";
        return false;
    }

    // common condition : LQ > 0
    if (!is_positive_number(LQ)) {
        log::debug(LOG_HEADER) << "attenuation param error: LQ <= 0";
        return false;
    }

    // common condition : LP > 0
    if (!is_positive_number(LP)) {
        log::debug(LOG_HEADER) << "attenuation param error: LP <= 0";
        return false;
    }

    // UN > 0
    if (!is_positive_number(UN)) {
        log::debug(LOG_HEADER) << "attenuation param error: UN <= 0";
        return false;
    }

    return true;
}

bool attenuation_model::check_model_param_un(attenuation_model& parser)
{
    auto LQ = parser.get_locked_quantity();
    auto LP = parser.get_locked_period();
    auto UN = parser.get_unlock_number();

    // given LQ, LP, UN, then
    // LP >= UN and LQ >= UN
    if (LP < UN) {
        log::debug(LOG_HEADER) << "attenuation param error: LP < UN";
        return false;
    }
    if (LQ < UN) {
        log::debug(LOG_HEADER) << "attenuation param error: LQ < UN";
        return false;
    }

    return true;
}

bool attenuation_model::check_model_param_uc_uq(attenuation_model& parser)
{
    auto LQ = parser.get_locked_quantity();
    auto LP = parser.get_locked_period();
    auto UN = parser.get_unlock_number();

    if (UN > max_unlock_number) {
        log::debug(LOG_HEADER) << "attenuation param error: UN > 100, at most 100 cycles is supported.";
        return false;
    }

    // given LQ, LP, UN, UC, UQ, then
    // LQ = sum(UQ) and all UQ > 0 and UQ.size == UN
    // LP = sum(UC) and all UC > 0 and UC.size == UN
    const auto& UCs = parser.get_unlock_cycles();
    const auto& UQs = parser.get_unlocked_quantities();
    if (UCs.size() != UN) {
        log::debug(LOG_HEADER) << "attenuation param error: UC.size() != UN";
        return false;
    }
    if (UQs.size() != UN) {
        log::debug(LOG_HEADER) << "attenuation param error: UQ.size() != UN";
        return false;
    }
    if (sum_and_check_numbers(UCs, is_positive_number) != LP) {
        log::debug(LOG_HEADER) << "attenuation param error: LP != sum(UC) or exist UC <= 0";
        return false;
    }
    if (sum_and_check_numbers(UQs, is_positive_number) != LQ) {
        log::debug(LOG_HEADER) << "attenuation param error: LQ != sum(UQ) or exist UQ <= 0";
        return false;
    }
    return true;
}

bool attenuation_model::check_model_param_inflation(attenuation_model& parser, uint64_t total_amount)
{
    if (!check_model_param_un(parser)) {
        return false;
    }

    auto LQ = parser.get_locked_quantity();
    auto UN = parser.get_unlock_number();
    auto IR = parser.get_inflation_rate();

    if (LQ != total_amount) {
        log::debug(LOG_HEADER) << "fixed inflation param error: partial lock is not supported!";
        return false;
    }

    if (UN > max_unlock_number) {
        log::debug(LOG_HEADER) << "fixed inflation param error: UN > 100, at most 100 cycles is supported.";
        return false;
    }

    // IR > 0
    if (IR <= 0 || IR > max_inflation_rate) {
        log::debug(LOG_HEADER) << "fixed inflation param error: IR not in [1, " << max_inflation_rate << "]";
        return false;
    }

    return true;
}

bool attenuation_model::check_model_param_initial_fixed_inflation(
    std::string& param, uint64_t total_amount, attenuation_model& parser, bool is_init)
{
    if (!check_model_param_inflation(parser, total_amount)) {
        return false;
    }

    auto PN = parser.get_current_period_number();
    auto LH = parser.get_latest_lock_height();
    auto LQ = parser.get_locked_quantity();
    auto LP = parser.get_locked_period();
    auto UN = parser.get_unlock_number();
    auto IR = parser.get_inflation_rate();

    if (LQ == total_amount) {
        uint64_t IP = LP / UN;

        // check LH after ensured UN > 0
        bool has_prefix = param.find("PN=") == 0;
        if (!has_prefix) {
            LH = IP;
        }
        else if (LH != IP) {
            log::debug(LOG_HEADER) << "fixed inflation param error: LH != " << IP;
            return false;
        }

        if (!is_init) {
            return check_model_param_uc_uq(parser);
        }

        // compute UC and UQ array
        double rate = IR / 100.0;
        double UP1= std::pow(1 + rate, int64_t(1 - UN));
        uint64_t UQ1 = LQ * UP1;
        UQ1 = std::min(UQ1, LQ);
        BITCOIN_ASSERT(UQ1 > 0);

        std::vector<uint64_t> uc_vec, uq_vec;
        uc_vec.push_back(IP);
        uq_vec.push_back(UQ1);

        uint64_t total_uc = IP;
        uint64_t total_uq = UQ1;
        uint64_t current_uc, current_uq;
        for (uint64_t i = 1; i < UN; ++i) {
            current_uq = total_uq * rate;
            current_uc = IP;

            if (i == UN - 1) {
                current_uc = LP - total_uc;
                current_uq = LQ - total_uq;
            }

            uc_vec.push_back(current_uc);
            uq_vec.push_back(current_uq);
            total_uc += current_uc;
            total_uq += current_uq;
        }

        BITCOIN_ASSERT(total_uc == LP);
        BITCOIN_ASSERT(total_uq == LQ);

        // rebuild parameter string
        // sample: PN=0;LH=2000;TYPE=3;LQ=9001;LP=6000;UN=3;IR=8
        std::stringstream ss;
        ss << "PN=0;LH=";   ss << std::to_string(LH);
        ss << ";TYPE=3;LQ="; ss << std::to_string(LQ);
        ss << ";LP="; ss << std::to_string(LP);
        ss << ";UN="; ss << std::to_string(UN);
        ss << ";IR="; ss << std::to_string(IR);
        ss << ";UC=";
        for (auto it = uc_vec.begin(); it != uc_vec.end(); ++it) {
            if (it != uc_vec.begin()) {
                ss << ",";
            }
            ss << std::to_string(*it);
        }

        ss << ";UQ=";
        for (auto it = uq_vec.begin(); it != uq_vec.end(); ++it) {
            if (it != uq_vec.begin()) {
                ss << ",";
            }
            ss << std::to_string(*it);
        }

        // update param
        ss >> param;

        return true;
    }

    else {
        log::error(LOG_HEADER) << "fixed inflation param error: partial lock is not supported!";
    }

    return false;
}

bool attenuation_model::validate_model_param(const data_chunk& param, uint64_t total_amount)
{
    attenuation_model parser(std::string(param.begin(), param.end()));

    const auto model = parser.get_model_type();

    // model_type::none is equivalent to
    // the scrpit pattern is not pay_key_hash_with_attenuation_model
    if (model == model_type::none) {
        if (!param.empty()) {
            log::debug(LOG_HEADER)
                << "check_model_param, wrong model param : "
                << parser.get_model_param();
        }
        return false;
    }

    auto PN = parser.get_current_period_number();
    auto LH = parser.get_latest_lock_height();
    auto LQ = parser.get_locked_quantity();
    auto LP = parser.get_locked_period();
    auto UN = parser.get_unlock_number();

    if (!check_model_param_common(parser)) {
        return false;
    }

    // common condition : LH > 0
    if (!is_positive_number(LH)) {
        log::debug(LOG_HEADER) << "common param error: LH <= 0";
        return false;
    }

    if (model == model_type::fixed_quantity) {
        uint64_t UC = LP / UN;
        if (PN + 1 == UN) { // last cycle
            if (PN * UC + LH > LP) {
                log::debug(LOG_HEADER) << "fixed cycle param error: last cycle PN * UC + LH > LP";
                return false;
            }
        } else {
            if (LH > UC) {
                log::debug(LOG_HEADER) << "fixed cycle param error: LH > UC";
                return false;
            }
        }
    }

    else if (model == model_type::custom) {
        const auto& UCs = parser.get_unlock_cycles();
        if (LH > UCs[PN]) {
            log::debug(LOG_HEADER) << "custom param error: LH > UC";
            return false;
        }
    }

    else if (model == model_type::fixed_inflation) {
        if (!check_model_param_inflation(parser, total_amount)) {
            return false;
        }

        if (!check_model_param_uc_uq(parser)) {
            return false;
        }

        const auto& UCs = parser.get_unlock_cycles();
        if (LH > UCs[PN]) {
            log::debug(LOG_HEADER) << "fixed inflation param error: LH > UC";
            return false;
        }
    }

    return true;
}

code attenuation_model::check_model_param(const blockchain::validate_transaction& validate_tx)
{
    const transaction& tx = validate_tx.get_tx();
    const blockchain::block_chain_impl& chain = validate_tx.get_blockchain();

    if (tx.version < transaction_version::check_nova_feature) {
        return error::success;
    }

    struct ext_input_point {
        chain::input_point input_point_;
        chain::output prev_output_;
        uint64_t prev_blockheight_;
    };

    std::vector<ext_input_point> vec_prev_input;

    for (const auto& input: tx.inputs) {
        if (input.previous_output.is_null()) {
            return error::previous_output_null;
        }

        chain::transaction prev_tx;
        uint64_t prev_height = 0;
        if (!validate_tx.get_previous_tx(prev_tx, prev_height, input)) {
            continue;
        }

        const auto& prev_output = prev_tx.outputs.at(input.previous_output.index);
        if (!operation::is_pay_key_hash_with_attenuation_model_pattern(prev_output.script.operations)) {
            continue;
        }

        ext_input_point prev{input.previous_output, prev_output, prev_height};
        vec_prev_input.emplace_back(prev);
    }

    if (vec_prev_input.empty()) {
        return error::success;
    }

    uint64_t current_blockheight = 0;
    chain.get_last_height(current_blockheight);

    for(auto& output : tx.outputs) {
        if (!operation::is_pay_key_hash_with_attenuation_model_pattern(output.script.operations)) {
            continue;
        }

        const auto& model_param = output.get_attenuation_model_param();
        if (!attenuation_model::validate_model_param(model_param, output.get_asset_amount())) {
            log::debug(LOG_HEADER) << "check param failed, " << chunk_to_string(model_param);
            return error::attenuation_model_param_error;
        }

        const auto& input_point_data = operation::
            get_input_point_from_pay_key_hash_with_attenuation_model(output.script.operations);
        chain::input_point input_point = chain::point::factory_from_data(input_point_data);
        if (input_point.is_null()) {
            if (!check_model_param(model_param, output.get_asset_amount())) {
                log::debug(LOG_HEADER) << "input is null, " << chunk_to_string(model_param);
                return error::attenuation_model_param_error;
            }
            continue;
        }

        auto iter = std::find_if(vec_prev_input.begin(), vec_prev_input.end(),
            [&input_point](const ext_input_point& elem){
                return elem.input_point_ == input_point;
            });

        if (iter == vec_prev_input.end()) {
            log::debug(LOG_HEADER) << "input not found for " << input_point.to_string();
            return error::attenuation_model_param_error;
        }

        const auto& prev_model_param = iter->prev_output_.get_attenuation_model_param();

        if (!check_model_param_immutable(prev_model_param, model_param)) {
            log::debug(LOG_HEADER) << "check immutable failed, "
                << "prev is " << chunk_to_string(prev_model_param)
                << ", new is " << chunk_to_string(model_param);
            return error::attenuation_model_param_error;
        }

        auto curr_diff_height = current_blockheight - iter->prev_blockheight_;
        auto real_diff_height = get_diff_height(prev_model_param, model_param);

        if (real_diff_height > curr_diff_height) {
            log::debug(LOG_HEADER) << "check diff height failed, "
                << real_diff_height << ", curr diff is" << curr_diff_height;
            return error::attenuation_model_param_error;
        }

        auto asset_total_amount = iter->prev_output_.get_asset_amount();
        auto new_model_param_ptr = std::make_shared<data_chunk>();
        auto asset_amount = attenuation_model::get_available_asset_amount(
                asset_total_amount, real_diff_height,
                prev_model_param, new_model_param_ptr);

        if (asset_total_amount != (asset_amount + output.get_asset_amount())) {
            log::debug(LOG_HEADER) << "check amount failed, "
                << "locked shoule be " << asset_total_amount - asset_amount
                << ", but real locked is " << output.get_asset_amount();
            return error::attenuation_model_param_error;
        }

        if (!new_model_param_ptr || (*new_model_param_ptr != model_param)) {
            log::debug(LOG_HEADER) << "check model new param failed, "
                << "prev is " << chunk_to_string(model_param) << ", new is "
                << (new_model_param_ptr ? chunk_to_string(*new_model_param_ptr) : std::string("empty"));
            return error::attenuation_model_param_error;
        }

        // prevent multiple locked outputs connect to the same input
        vec_prev_input.erase(iter);
    }

    // check the left is all spendable
    for (const auto& ext_input : vec_prev_input) {
        const auto& prev_model_param = ext_input.prev_output_.get_attenuation_model_param();
        auto curr_diff_height = current_blockheight - ext_input.prev_blockheight_;
        auto real_diff_height = get_diff_height(prev_model_param, data_chunk());
        if (real_diff_height > curr_diff_height) {
            log::debug(LOG_HEADER) << "check diff height failed for all spendable, "
                << real_diff_height << ", curr diff is" << curr_diff_height;
            return error::attenuation_model_param_error;
        }
    }

    return error::success;
}

uint64_t attenuation_model::get_diff_height(const data_chunk& prev_param, const data_chunk& param)
{
    attenuation_model parser(std::string(prev_param.begin(), prev_param.end()));
    auto model = parser.get_model_type();
    if (model == model_type::none) {
        return max_uint64;
    }

    auto PN = parser.get_current_period_number();
    auto LH = parser.get_latest_lock_height();
    auto LP = parser.get_locked_period();
    auto UN = parser.get_unlock_number();

    uint64_t PN2 = 0;
    uint64_t LH2 = 0;
    if (!param.empty()) {
        attenuation_model parser2(std::string(param.begin(), param.end()));
        PN2 = parser2.get_current_period_number();
        LH2 = parser2.get_latest_lock_height();
    } else {
        PN2 = UN - 1;
        LH2 = 0;
    }

    if (PN > PN2) {
        return max_uint64;
    }

    if (PN == PN2) {
        if (LH <= LH2) {
            return max_uint64;
        }
        return LH - LH2;
    }

    if (model == model_type::fixed_quantity) {
        auto UC = LP / UN;
        if (PN2 + 1 == UN) {
            return LP - LH2 - ((PN + 1) * UC) + LH;
        }
        return LH + ((PN2 - PN) * UC) - LH2;
    }

    else if (model == model_type::custom || model == model_type::fixed_inflation) {
        const auto& UCs = parser.get_unlock_cycles();
        auto diff_height = LH;
        for (auto i = PN + 1; i <= PN2; ++i) {
            diff_height += UCs[i];
        }
        diff_height -= LH2;
        return diff_height;
    }

    return 0;
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
        if (!param.empty()) {
            log::debug(LOG_HEADER)
                << "get_available_asset_amount, wrong model param : "
                << parser.get_model_param();
        }
        return asset_amount;
    }

    auto PN = parser.get_current_period_number();
    auto LH = parser.get_latest_lock_height();
    auto LQ = parser.get_locked_quantity();
    auto LP = parser.get_locked_period();
    auto UN = parser.get_unlock_number();
    const auto& UCs = parser.get_unlock_cycles();
    const auto& UQs = parser.get_unlocked_quantities();

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
        uint64_t UC = LP / UN;
        auto elapsed_height = (diff_height - LH) + ((PN + 1) * UC);
        if (elapsed_height >= LP) { // include the last unlock cycle, release all
            return asset_amount;
        }
        auto new_cycles = std::min((elapsed_height / UC - PN), (LP - PN - 1));
        if (new_param_ptr) {
            // update PN, LH
            PN = PN + new_cycles;
            if ((PN + 1) == UN) { // last cycle
                LH = LP - elapsed_height;
            } else {
                LH = (PN + 1) * UC - elapsed_height;
            }
            *new_param_ptr = parser.get_new_model_param(PN, LH);
        }
        auto UQ = LQ / UN;
        available += new_cycles * UQ;
        return available;
    }

    if (model == model_type::custom || model == model_type::fixed_inflation) {
        available += UQs[PN];
        diff_height -= LH;
        ++PN;
        while ((PN < UN) && (diff_height >= UCs[PN])) {
            available += UQs[PN];
            diff_height -= UCs[PN];
            ++PN;
        }
        if (PN == UN) { // include the last unlock cycle, release all
            return asset_amount;
        }
        if (new_param_ptr) {
            // update PN, LH
            LH = UCs[PN] - diff_height;
            *new_param_ptr = parser.get_new_model_param(PN, LH);
        }
        return available;
    }

    log::debug(LOG_HEADER) << "get_available_asset_amount, Unsupported attenuation model: "
        << std::to_string(to_index(model));
    return asset_amount;
}

} // namspace chain
} // namspace libbitcoin
