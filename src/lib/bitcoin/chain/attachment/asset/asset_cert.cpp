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
             || (calc_size() > ASSET_CERT_FIX_SIZE));
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

std::string asset_cert::get_key(const std::string&symbol, const asset_cert_type& bit)
{
    return std::string(symbol + ":^#`@:" + std::to_string(bit));
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
    cert_type_ = source.read_4_bytes_little_endian();
    status_ = source.read_byte();

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
    sink.write_4_bytes_little_endian(cert_type_);
    sink.write_byte(status_);
}

uint64_t asset_cert::calc_size() const
{
    return (symbol_.size() + 1)
        + (owner_.size() + 1)
        + (address_.size() + 1)
        + ASSET_CERT_TYPE_FIX_SIZE
        + ASSET_CERT_STATUS_FIX_SIZE;
}

uint64_t asset_cert::serialized_size() const
{
    return std::min<uint64_t>(calc_size(), ASSET_CERT_FIX_SIZE);
}

std::string asset_cert::to_string() const
{
    std::ostringstream ss;
    ss << "\t symbol = " << symbol_ << "\n";
    ss << "\t owner = " << owner_ << "\n";
    ss << "\t address = " << address_ << "\n";
    ss << "\t cert = " << get_type_name() << "\n";
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

asset_cert_type asset_cert::get_type() const
{
    return cert_type_;
}

void asset_cert::set_type(asset_cert_type cert_type)
{
    cert_type_ = cert_type;
}

asset_cert_type asset_cert::get_certs() const
{
    return cert_type_;
}

void asset_cert::set_certs(asset_cert_type cert_type)
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

bool asset_cert::is_movalbe(asset_cert_type cert_type)
{
    return (cert_type | asset_cert_ns::unmovable_flag) != 0;
}

bool asset_cert::is_movalbe() const
{
    return asset_cert::is_movalbe(cert_type_);
}

} // namspace chain
} // namspace libbitcoin
