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
    cert_type_ = asset_cert_ns::none;
    status_ = ASSET_CERT_NORMAL_TYPE;
}

bool asset_cert::is_valid() const
{
    return !(symbol_.empty()
            || owner_.empty()
            || (cert_type_ == asset_cert_ns::none)
            || ((symbol_.size()+1) > ASSET_CERT_SYMBOL_FIX_SIZE)
            || ((owner_.size()+1) > ASSET_CERT_OWNER_FIX_SIZE)
            || ((address_.size()+1) > ASSET_CERT_ADDRESS_FIX_SIZE)
            );
}

bool asset_cert::operator< (const asset_cert& other) const
{
    auto ret = symbol_.compare(other.symbol_);
    if (ret < 0) {
        return true;
    }
    else if (ret == 0) {
        if (cert_type_ < other.cert_type_) {
            return true;
        }
        else if (cert_type_ == other.cert_type_) {
            return address_.compare(other.address_) < 0;
        }
    }

    return false;
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

std::string asset_cert::get_key(const std::string&symbol, asset_cert_type bit)
{
    return std::string(symbol + ":^#`@:" + std::to_string(bit));
}

std::string asset_cert::asset_cert::get_key() const
{
    return get_key(symbol_, cert_type_);
}

asset_cert asset_cert::factory_from_data(const data_chunk& data)
{
    asset_cert instance;
    instance.from_data(data);
    return instance;
}

asset_cert asset_cert::factory_from_data(std::istream& stream)
{
    asset_cert instance;
    instance.from_data(stream);
    return instance;
}

asset_cert asset_cert::factory_from_data(reader& source)
{
    asset_cert instance;
    instance.from_data(source);
    return instance;
}

bool asset_cert::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool asset_cert::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool asset_cert::from_data(reader& source)
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

data_chunk asset_cert::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    return data;
}

void asset_cert::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void asset_cert::to_data(writer& sink) const
{
    sink.write_string(symbol_);
    sink.write_string(owner_);
    sink.write_string(address_);
    sink.write_4_bytes_little_endian(cert_type_);
    sink.write_byte(status_);
}

uint64_t asset_cert::serialized_size() const
{
    size_t len = (symbol_.size()+1) + (owner_.size()+1) + (address_.size()+1)
        + ASSET_CERT_TYPE_FIX_SIZE + ASSET_CERT_STATUS_FIX_SIZE;
    return std::min(len, ASSET_CERT_FIX_SIZE);
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
    size_t len = std::min((symbol.size()+1), ASSET_CERT_SYMBOL_FIX_SIZE);
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
    size_t len = std::min((owner.size()+1), ASSET_CERT_OWNER_FIX_SIZE);
    owner_ = owner.substr(0, len);
}

const std::string& asset_cert::get_address() const
{
    return address_;
}

void asset_cert::set_address(const std::string& address)
{
    size_t len = std::min((address.size()+1), ASSET_CERT_ADDRESS_FIX_SIZE);
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
    };
    return static_type_name_map;
}

std::string asset_cert::get_type_name(asset_cert_type cert_type)
{
    BITCOIN_ASSERT(cert_type != asset_cert_ns::none);

    const auto& type_name_map = get_type_name_map();
    auto iter = type_name_map.find(cert_type);
    BITCOIN_ASSERT(iter != type_name_map.end());
    return iter->second;
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


} // namspace chain
} // namspace libbitcoin
