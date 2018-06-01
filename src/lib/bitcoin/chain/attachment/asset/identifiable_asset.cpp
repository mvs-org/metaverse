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
#include <metaverse/bitcoin/chain/attachment/asset/identifiable_asset.hpp>
#include <sstream>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>
#include <metaverse/blockchain/block_chain_impl.hpp>
#include <metaverse/blockchain/validate_transaction.hpp>
#include <metaverse/mgbubble/utility/Compare.hpp>

namespace libbitcoin {
namespace chain {

identifiable_asset::identifiable_asset()
{
    reset();
}

identifiable_asset::identifiable_asset(const std::string& symbol,
                                       const std::string& address, const std::string& content)
    : symbol_(symbol)
    , address_(address)
    , content_(content)
    , status_(IDENTIFIABLE_ASSET_NORMAL_TYPE)
{
}

void identifiable_asset::reset()
{
    symbol_ = "";
    address_ = "";
    content_ = "";
    status_ = IDENTIFIABLE_ASSET_NORMAL_TYPE;
}

bool identifiable_asset::is_valid() const
{
    return !(symbol_.empty()
             || address_.empty()
             || ((symbol_.size() + 1) > IDENTIFIABLE_ASSET_SYMBOL_FIX_SIZE)
             || ((address_.size() + 1) > IDENTIFIABLE_ASSET_ADDRESS_FIX_SIZE)
             || ((content_.size() + 1) > IDENTIFIABLE_ASSET_CONTENT_FIX_SIZE)
            );
}

bool identifiable_asset::operator< (const identifiable_asset& other) const
{
    return symbol_.compare(other.symbol_) < 0;
}

identifiable_asset identifiable_asset::factory_from_data(const data_chunk& data)
{
    identifiable_asset instance;
    instance.from_data(data);
    return instance;
}

identifiable_asset identifiable_asset::factory_from_data(std::istream& stream)
{
    identifiable_asset instance;
    instance.from_data(stream);
    return instance;
}

identifiable_asset identifiable_asset::factory_from_data(reader& source)
{
    identifiable_asset instance;
    instance.from_data(source);
    return instance;
}

bool identifiable_asset::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool identifiable_asset::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool identifiable_asset::from_data(reader& source)
{
    reset();

    symbol_ = source.read_string();
    address_ = source.read_string();
    content_ = source.read_string();
    status_ = source.read_byte();

    auto result = static_cast<bool>(source);
    if (!result)
        reset();

    return result;
}

data_chunk identifiable_asset::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    return data;
}

void identifiable_asset::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void identifiable_asset::to_data(writer& sink) const
{
    sink.write_string(symbol_);
    sink.write_string(address_);
    sink.write_string(content_);
    sink.write_byte(status_);
}

uint64_t identifiable_asset::serialized_size() const
{
    size_t len = (symbol_.size() + 1) + (address_.size() + 1) + (content_.size() + 1)
                 + IDENTIFIABLE_ASSET_STATUS_FIX_SIZE;
    return std::min(len, IDENTIFIABLE_ASSET_FIX_SIZE);
}

std::string identifiable_asset::to_string() const
{
    std::ostringstream ss;
    ss << "\t symbol = " << symbol_ << "\n";
    ss << "\t address = " << address_ << "\n";
    ss << "\t content = " << content_ << "\n";
    ss << "\t status = " << std::to_string(status_) << "\n";
    return ss.str();
}

const std::string& identifiable_asset::get_symbol() const
{
    return symbol_;
}

void identifiable_asset::set_symbol(const std::string& symbol)
{
    size_t len = std::min((symbol.size() + 1), IDENTIFIABLE_ASSET_SYMBOL_FIX_SIZE);
    symbol_ = symbol.substr(0, len);
}

const std::string& identifiable_asset::get_address() const
{
    return address_;
}

void identifiable_asset::set_address(const std::string& address)
{
    size_t len = std::min((address.size() + 1), IDENTIFIABLE_ASSET_ADDRESS_FIX_SIZE);
    address_ = address.substr(0, len);
}

const std::string& identifiable_asset::get_content() const
{
    return content_;
}

void identifiable_asset::set_content(const std::string& content)
{
    size_t len = std::min((content.size() + 1), IDENTIFIABLE_ASSET_CONTENT_FIX_SIZE);
    content_ = content.substr(0, len);
}

uint8_t identifiable_asset::get_status() const
{
    return status_;
}

void identifiable_asset::set_status(uint8_t status)
{
    status_ = status;
}

} // namspace chain
} // namspace libbitcoin
