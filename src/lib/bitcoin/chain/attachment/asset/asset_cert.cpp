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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/bitcoin/chain/attachment/asset/asset_cert.hpp>
#include <sstream>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>
#include <metaverse/blockchain/block_chain_impl.hpp>
#include <metaverse/blockchain/validate_transaction.hpp>

namespace libbitcoin {
namespace chain {

#define ASSET_SYMBOL_DELIMITER "."

std::istream& operator>>(std::istream& in, asset_cert_type& out){
    uint32_t & cert_type = out.mask;
    in >> cert_type;
    return in;
}

const std::string asset_cert::key_initial{"initial"};
const std::string asset_cert::key_interval{"interval"};
const std::string asset_cert::key_base{"base"};

asset_cert::asset_cert()
{
    reset();
}

asset_cert::asset_cert(const std::string& symbol, const std::string& owner,
                       const std::string& address, asset_cert_type cert_type)
    : symbol_(symbol)
    , owner_(owner)
    , address_(address)
    , cert_type_(cert_type)
    , status_(ASSET_CERT_NORMAL_TYPE)
{
}

void asset_cert::reset()
{
    symbol_ = "";
    owner_ = "";
    address_ = "";
    cert_type_ = asset_cert_ns::none;
    status_ = ASSET_CERT_NORMAL_TYPE;
}

bool asset_cert::is_valid() const
{
    return !(symbol_.empty()
             || owner_.empty()
             || (cert_type_ == asset_cert_ns::none)
             || (calc_size() > (has_content() ? ASSET_CERT_FULL_FIX_SIZE : ASSET_CERT_FIX_SIZE)));
}

bool asset_cert::operator< (const asset_cert& other) const
{
    typedef std::tuple<std::string, asset_cert_type> cmp_tuple;
    return cmp_tuple(symbol_, cert_type_) < cmp_tuple(other.symbol_, other.cert_type_);
}

std::string asset_cert::get_domain(const std::string& symbol)
{
    std::string domain("");
    auto&& tokens = bc::split(symbol, ASSET_SYMBOL_DELIMITER, true);
    if (tokens.size() > 0) {
        domain = tokens[0];
    }
    return domain;
}

bool asset_cert::is_valid_domain(const std::string& domain)
{
    return !domain.empty();
}

std::string asset_cert::get_key(const std::string& symbol, const asset_cert_type& bit)
{
    return std::string(symbol + ":^#`@:" + std::to_string(bit));
}

std::string asset_cert::get_witness_key(const std::string& symbol)
{
    return std::string(symbol + ":^#`@:" + std::to_string(asset_cert_ns::witness));
}

std::string asset_cert::asset_cert::get_key() const
{
    return get_key(symbol_, cert_type_);
}

bool asset_cert::from_data_t(reader& source)
{
    reset();
    symbol_ = source.read_string();
    owner_ = source.read_string();
    address_ = source.read_string();
    cert_type_.mask = source.read_4_bytes_little_endian();
    status_ = source.read_byte();

    if (has_content()) {
        content_ = source.read_string();
    }

    auto result = static_cast<bool>(source);
    if (!result)
        reset();

    return result;
}

void asset_cert::to_data_t(writer& sink) const
{
    sink.write_string(symbol_);
    sink.write_string(owner_);
    sink.write_string(address_);
    sink.write_4_bytes_little_endian(cert_type_.mask);
    sink.write_byte(status_);

    if (has_content()) {
        sink.write_string(content_);
    }
}

uint64_t asset_cert::calc_size() const
{
    auto size = (symbol_.size() + 1) + (owner_.size() + 1) + (address_.size() + 1)
        + ASSET_CERT_TYPE_FIX_SIZE + ASSET_CERT_STATUS_FIX_SIZE;

    if (has_content()) {
        size += (content_.size() + 1);
    }

    return size;
}

uint64_t asset_cert::serialized_size() const
{
    if (has_content()) {
        std::min<uint64_t>(calc_size(), ASSET_CERT_FULL_FIX_SIZE);
    }

    return std::min<uint64_t>(calc_size(), ASSET_CERT_FIX_SIZE);
}

std::string asset_cert::to_string() const
{
    std::ostringstream ss;
    ss << "\t symbol = " << symbol_ << "\n";
    ss << "\t owner = " << owner_ << "\n";
    ss << "\t address = " << address_ << "\n";
    ss << "\t cert = " << get_type_name() << "\n";
    if (!content_.empty()) {
        ss << "\t description = " << content_ << "\n";
    }
    ss << "\t status = " << std::to_string(status_) << "\n";
    return ss.str();
}

const std::string& asset_cert::get_symbol() const
{
    return symbol_;
}

void asset_cert::set_symbol(const std::string& symbol)
{
    size_t len = std::min((symbol.size() + 1), ASSET_CERT_SYMBOL_FIX_SIZE);
    symbol_ = symbol.substr(0, len);
}

uint8_t asset_cert::get_status() const
{
    return status_;
}

void asset_cert::set_status(uint8_t status)
{
    status_ = status;
}

bool asset_cert::is_newly_generated() const
{
    return (status_ == ASSET_CERT_ISSUE_TYPE)
           || (status_ == ASSET_CERT_AUTOISSUE_TYPE);
}

const std::string& asset_cert::get_owner() const
{
    return owner_;
}

void asset_cert::set_owner(const std::string& owner)
{
    size_t len = std::min((owner.size() + 1), ASSET_CERT_OWNER_FIX_SIZE);
    owner_ = owner.substr(0, len);
}

const std::string& asset_cert::get_address() const
{
    return address_;
}

void asset_cert::set_address(const std::string& address)
{
    size_t len = std::min((address.size() + 1), ASSET_CERT_ADDRESS_FIX_SIZE);
    address_ = address.substr(0, len);
}

const std::string& asset_cert::get_content() const
{
    return content_;
}

void asset_cert::set_content(const std::string& content)
{
    size_t len = std::min((content.size() + 1), ASSET_CERT_CONTENT_FIX_SIZE);
    content_ = content.substr(0, len);
}

asset_cert_type asset_cert::get_type() const
{
    return cert_type_;
}

void asset_cert::set_type(asset_cert_type cert_type)
{
    cert_type_ = cert_type;
}

std::string asset_cert::get_type_name() const
{
    return get_type_name(cert_type_);
}

const std::map<asset_cert_type, std::string>& asset_cert::get_type_name_map()
{
    static std::map<asset_cert_type, std::string> static_type_name_map = {
        {asset_cert_ns::issue, "issue"},
        {asset_cert_ns::domain, "domain"},
        {asset_cert_ns::naming, "naming"},
        {asset_cert_ns::mining, "mining"},
        {asset_cert_ns::witness, "witness"},

        {asset_cert_ns::marriage,   "marriage"},
        {asset_cert_ns::kyc,        "KYC"},
    };
    return static_type_name_map;
}

std::string asset_cert::get_type_name(asset_cert_type cert_type)
{
    BITCOIN_ASSERT(cert_type != asset_cert_ns::none);

    const auto& type_name_map = get_type_name_map();
    auto iter = type_name_map.find(cert_type);
    if (iter != type_name_map.end()) {
        return iter->second;
    }

    std::stringstream sstream;
    sstream << "0x" << std::hex << cert_type;
    std::string result = sstream.str();
    return result;
}

bool asset_cert::test_certs(const std::vector<asset_cert_type>& cert_vec, asset_cert_type cert_type)
{
    BITCOIN_ASSERT(cert_type != asset_cert_ns::none);

    auto iter = std::find(cert_vec.begin(), cert_vec.end(), cert_type);
    return iter != cert_vec.end();
}

bool asset_cert::test_certs(const std::vector<asset_cert_type>& total, const std::vector<asset_cert_type>& parts)
{
    if (total.size() < parts.size()) {
        return false;
    }

    for (auto& cert_type : parts) {
        if (!test_certs(total, cert_type)) {
            return false;
        }
    }

    return true;
}

bool asset_cert::has_content(asset_cert_type cert_type)
{
    return cert_type.has_content();
}

bool asset_cert::has_content() const
{
    return cert_type_.has_content();
}

std::vector<std::string> asset_cert::get_mining_subsidy_param_keys()
{
    std::vector<std::string> keys{ key_initial, key_interval, key_base };
    return keys;
}

bool asset_cert::check_mining_subsidy_param() const
{
    if (cert_type_ != asset_cert_ns::mining) {
        return true;
    }

    return get_mining_subsidy_param() != nullptr;
}

asset_cert::mining_subsidy_param_ptr asset_cert::get_mining_subsidy_param() const
{
    return parse_mining_subsidy_param(content_);
}

asset_cert::mining_subsidy_param_ptr asset_cert::parse_mining_subsidy_param(const std::string& param)
{
    if (param.empty()) {
        log::error("cert") << "mining cert: subsidy parameter is empty!";
        return nullptr;
    }

    if (param.size() > ASSET_CERT_CONTENT_FIX_SIZE) {
        log::error("cert") << "mining cert: subsidy parameter is out of range. Max size is 64.";
        return nullptr;
    }

    std::vector<std::string> items = bc::split(param, ",", true);
    if (items.size() < 3) {
        log::error("cert") << "mining cert: invalid size of parameters: " << param;
        return nullptr;
    }

    auto&& keys = get_mining_subsidy_param_keys();
    std::map<std::string, std::string> params;
    for (auto& item : items) {
        auto pair = bc::split(item, ":", true);
        if (pair.size() != 2) {
            log::error("cert") << "mining cert: invalid item " << item;
            return nullptr;
        }

        auto key = pair[0];
        auto value = pair[1];
        if (std::find(std::begin(keys), std::end(keys), key) != keys.end()) {
            params[key] = value;
        }
    }

    if (params.size() < keys.size()) {
        log::error("cert") << "mining cert: lack of parameter: " << param;
        return nullptr;
    }

    try {
        std::string value = params[key_initial];
        int32_t initial = boost::lexical_cast<int>(value);
        if (initial <= 0) {
            log::error("cert") << "mining cert: invalid initial subsidy parameter: " << value;
            return nullptr;
        }

        value = params[key_interval];
        int32_t interval = boost::lexical_cast<int>(value);
        if (interval <= 0) {
            log::error("cert") << "mining cert: invalid block interval parameter: " << value;
            return nullptr;
        }

        value = params[key_base];
        double base = boost::lexical_cast<double>(value);
        if (base <= 0) {
            log::error("cert") << "mining cert: invalid base parameter: " << value;
            return nullptr;
        }

        auto subsidy_params = std::make_shared<mining_subsidy_param_t>();
        (*subsidy_params)[key_initial] = initial;
        (*subsidy_params)[key_interval] = interval;
        (*subsidy_params)[key_base] = base;
        return subsidy_params;
    }
    catch (boost::bad_lexical_cast & e) {
        log::error("cert") << "mining cert: invalid value type: " << param << e.what();
        return nullptr;
    }

    return nullptr;
}

bool asset_cert::parse_uint32(const std::string& param, uint32_t& value)
{
    for (auto& i : param){
        if (!std::isalnum(i)) {
            return false;
        }
    }

    value = std::stoi(param);
    return true;
}

std::string asset_cert::get_primary_witness_symbol(const std::string& symbol)
{
    if (symbol.empty() || symbol.find(witness_cert_prefix) != 0) {
        return "";
    }

    auto offset = witness_cert_prefix.size();
    auto index_str = symbol.substr(offset);

    std::vector<std::string> items = bc::split(index_str, ".");
    if (items.size() < 1) {
        return "";
    }

    auto fmt = boost::format("%1%%2%") % witness_cert_prefix % items[0];
    return fmt.str();
}

bool asset_cert::is_valid_primary_witness(const std::string& symbol)
{
    if (symbol.empty() || symbol.find(witness_cert_prefix) != 0) {
        return false;
    }

    auto offset = witness_cert_prefix.size();
    auto index_str = symbol.substr(offset);

    uint32_t pri_index = 0;
    if (!parse_uint32(index_str, pri_index)) {
        return false;
    }

    return (pri_index >= 1 && pri_index <= witness_cert_count);
}

bool asset_cert::is_valid_secondary_witness(const std::string& symbol)
{
    if (symbol.empty() || symbol.find(witness_cert_prefix) != 0) {
        return false;
    }

    auto offset = witness_cert_prefix.size();
    auto index_str = symbol.substr(offset);

    std::vector<std::string> items = bc::split(index_str, ".");
    if (items.size() < 2 || items[1].empty()) {
        return false;
    }

    uint32_t pri_index = 0;
    if (!parse_uint32(items[0], pri_index)) {
        return false;
    }

    return (pri_index >= 1 && pri_index <= witness_cert_count);
}

uint32_t asset_cert::get_primary_witness_index(const std::string& symbol)
{
    if (symbol.empty() || symbol.find(witness_cert_prefix) != 0) {
        return 0;
    }

    auto offset = witness_cert_prefix.size();
    auto index_str = symbol.substr(offset);

    std::vector<std::string> items = bc::split(index_str, ".");
    if (items.size() < 1) {
        return 0;
    }

    uint32_t pri_index = 0;
    if (!parse_uint32(items[0], pri_index)) {
        return 0;
    }

    return pri_index;
}

bool asset_cert::is_primary_witness() const
{
    return (cert_type_ == asset_cert_ns::witness && is_valid_primary_witness(symbol_));
}

bool asset_cert::is_secondary_witness() const
{
    return (cert_type_ == asset_cert_ns::witness && is_valid_secondary_witness(symbol_));
}

} // namspace chain
} // namspace libbitcoin
