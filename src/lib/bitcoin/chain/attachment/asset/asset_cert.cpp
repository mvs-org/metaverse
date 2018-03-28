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
#include <boost/algorithm/string.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>
#include <metaverse/blockchain/block_chain_impl.hpp>
#include <metaverse/blockchain/validate_transaction.hpp>

namespace libbitcoin {
namespace chain {

// assert cert name definition
const char* asset_cert_name_none            = "NONE";
const char* asset_cert_name_secondary_issue = "ASI";
const char* asset_cert_name_all             = "ALL";
// name to type map
static std::unordered_map<std::string, asset_cert_type> cert_name_type_map{
    {asset_cert_name_none, asset_cert_ns::none},
    {asset_cert_name_secondary_issue, asset_cert_ns::secondary_issue},
    {asset_cert_name_all, asset_cert_ns::all}
};

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
            || (certs_ == asset_cert_ns::none)
            || ((symbol_.size()+1) > ASSET_CERT_SYMBOL_FIX_SIZE)
            || ((owner_.size()+1) > ASSET_CERT_OWNER_FIX_SIZE)
            );
}

bool asset_cert::operator< (const asset_cert& other) const
{
    return (symbol_ < other.symbol_)
        || ((symbol_ == other.symbol_) && (certs_ < other.certs_));
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
    ss << "\t certs = " << get_certs_name() << "\n";
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
    return test_certs(certs_, bits);
}

bool asset_cert::test_certs(asset_cert_type certs, asset_cert_type bits)
{
    return (certs & bits) == bits;
}

std::string asset_cert::get_certs_name() const
{
    return get_certs_name(certs_);
}

std::string asset_cert::get_certs_name(asset_cert_type certs)
{
    if (certs == asset_cert_ns::none) {
        return "NONE";
    }
    if (certs == asset_cert_ns::all) {
        return "ALL";
    }
    // collect cert names to a set container
    std::set<std::string> name_vec;
    if (test_certs(certs, asset_cert_ns::secondary_issue)) {
        name_vec.insert(asset_cert_name_secondary_issue);
    }

    // concat cert names, separated by comma
    std::string certs_name;
    for (auto iter = name_vec.begin(); iter != name_vec.end(); ++iter) {
        if (iter == name_vec.begin()) {
            certs_name += *iter;
        } else {
            certs_name += ("," + *iter);
        }
    }

    if (certs_name.empty()) {
        return "NONE";
    }

    return certs_name;
}

asset_cert_type asset_cert::get_certs_from_name(const std::string& certs_name)
{
    asset_cert_type certs = asset_cert_ns::none;
    std::vector<std::string> name_vec;
    boost::split(name_vec, certs_name, boost::is_any_of(","));
    for (const auto& name : name_vec) {
        auto iter = cert_name_type_map.find(name);
        if (iter != cert_name_type_map.end()) {
            certs |= iter->second;
        }
    }
    return certs;
}

// split input certs of this into two output parts d1 and d2, d1 with bits, d2 with the other.
bool asset_cert::split_certs(asset_cert& d1, asset_cert& d2, asset_cert_type bits) const
{
    if (!test_certs(bits) || (&d1 == this) || (&d2 == this)) {
        return false;
    }
    d1.set_symbol(symbol_);
    d2.set_symbol(symbol_);
    d1.set_certs(bits);
    d2.set_certs(certs_ & (~bits));
    return true;
}

bool asset_cert::check_cert_owner(bc::blockchain::block_chain_impl& chain) const
{
    // don't check if did is not enabled.
    if (!bc::blockchain::validate_transaction::is_did_validate(chain)) {
        return true;
    }
    auto did_symbol = owner_;
    return chain.get_issued_did(did_symbol) != nullptr;
}

code asset_cert::check_certs_split(
        const asset_cert_container& src,
        const asset_cert_container& dest,
        bc::blockchain::block_chain_impl& chain)
{
    if (src.empty() || dest.empty()) {
        return error::asset_cert_error;
    }
    bool did_enabled = bc::blockchain::validate_transaction::is_did_validate(chain);
    std::shared_ptr<std::vector<did_detail>> sp_issued_dids;
    if (did_enabled) {
        sp_issued_dids = chain.get_issued_dids();
    }
    auto check_owner = [did_enabled, &sp_issued_dids](std::string owner) {
        return (!did_enabled)
            || (std::find_if(sp_issued_dids->begin(), sp_issued_dids->end(),
                [&owner](did_detail& elem) {
                    return elem.get_symbol() == owner;
                })) != sp_issued_dids->end();
    };

    auto symbol = src.begin()->get_symbol();
    asset_cert_type src_types{0};
    asset_cert_type dest_types{0};
    for (auto& cert : src) {
        if (cert.get_symbol() != symbol) {
            return error::asset_cert_error;
        }
        if ((src_types & cert.get_certs()) != 0) {
            return error::asset_cert_error;
        }
        src_types |= cert.get_certs();
    }
    for (auto& cert : dest) {
        if (cert.get_symbol() != symbol) {
            return error::asset_cert_error;
        }
        if ((dest_types & cert.get_certs()) != 0) {
            return error::asset_cert_error;
        }
        if (!check_owner(cert.get_owner())) {
            return error::did_address_needed;
        }
        dest_types |= cert.get_certs();
    }
    if (src_types != dest_types ) {
        return error::asset_cert_error;
    }
    return error::success;
}

} // namspace chain
} // namspace libbitcoin
