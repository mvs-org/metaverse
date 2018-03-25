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

namespace libbitcoin {
namespace chain {

asset_cert::asset_cert()
{
    reset();
}

asset_cert::asset_cert(std::string symbol, std::string owner, asset_cert_type certs)
    : symbol_(symbol)
    , owner_(owner)
    , certs_(certs)
{
}

void asset_cert::reset()
{
    symbol_ = "";
    owner_ = "";
    certs_ = 0;
}

bool asset_cert::is_valid() const
{
    return !(symbol_.empty()
            || owner_.empty()
            || (certs_ == 0)
            || ((symbol_.size()+1) > ASSET_CERT_SYMBOL_FIX_SIZE)
            || ((owner_.size()+1) > ASSET_CERT_OWNER_FIX_SIZE)
            );
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
    certs_ = source.read_8_bytes_little_endian();

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
    sink.write_8_bytes_little_endian(certs_);
}

uint64_t asset_cert::serialized_size() const
{
    size_t len = (symbol_.size()+1) + (owner_.size()+1) + ASSET_CERT_CERTS_FIX_SIZE;
    return std::min(len, ASSET_CERT_FIX_SIZE);
}

std::string asset_cert::to_string() const
{
    std::ostringstream ss;
    ss << "\t symbol = " << symbol_ << "\n";
    ss << "\t owner = " << owner_ << "\n";
    ss << "\t certs = " << certs_ << "\n";
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

const std::string& asset_cert::get_owner() const
{
    return owner_;
}

void asset_cert::set_owner(const std::string& owner)
{
    size_t len = std::min((owner.size()+1), ASSET_CERT_OWNER_FIX_SIZE);
    owner_ = owner.substr(0, len);
}

asset_cert_type asset_cert::get_certs() const
{
    return certs_;
}

void asset_cert::set_certs(asset_cert_type certs)
{
    certs_ = certs;
}

bool asset_cert::test_certs(asset_cert_type bits) const
{
    return (certs_ & bits) == bits;
}

bool asset_cert::split_certs(asset_cert& d1, asset_cert& d2, asset_cert_type bits)
{
    if (!test_certs(bits)) {
        return false;
    }
    d1.set_symbol(symbol_);
    d2.set_symbol(symbol_);
    d1.set_certs(bits);
    d2.set_certs(certs_ & (~bits));
    return true;
}

bool asset_cert::check_certs_split(asset_cert s, asset_cert d1, asset_cert d2)
{
    return (s.certs_ == (d1.certs_ ^ d2.certs_))
        && (s.symbol_ == d1.symbol_)
        && (s.symbol_ == d2.symbol_);
}

} // namspace chain
} // namspace libbitcoin
