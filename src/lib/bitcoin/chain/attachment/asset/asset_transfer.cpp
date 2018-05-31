/**
 * Copyright (c) 2011-2015 metaverse developers (see AUTHORS)
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
#include <metaverse/bitcoin/chain/attachment/asset/asset_transfer.hpp>

#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>
#include <json/minijson_writer.hpp>
#include <metaverse/mgbubble/utility/Compare.hpp>

namespace libbitcoin {
namespace chain {

bool asset_balances::operator< (const asset_balances& other) const
{
    auto ret = 0;
    if ((ret = symbol.compare(other.symbol)) < 0
        || (ret == 0 && (ret = address.compare(other.address)) < 0)
        || (ret == 0 && (ret = mgbubble::compare(unspent_asset ,other.unspent_asset)) < 0)
        || (ret == 0 && mgbubble::compare(locked_asset ,other.locked_asset) < 0)
        )
        return true;

    return false;
}
    
asset_transfer::asset_transfer()
{
    reset();
}
asset_transfer::asset_transfer(const std::string& symbol, uint64_t quantity):
    symbol(symbol),quantity(quantity)
{

}
asset_transfer asset_transfer::factory_from_data(const data_chunk& data)
{
    asset_transfer instance;
    instance.from_data(data);
    return instance;
}

asset_transfer asset_transfer::factory_from_data(std::istream& stream)
{
    asset_transfer instance;
    instance.from_data(stream);
    return instance;
}

asset_transfer asset_transfer::factory_from_data(reader& source)
{
    asset_transfer instance;
    instance.from_data(source);
    return instance;
}

bool asset_transfer::is_valid() const
{
    return !(symbol.empty()
            || quantity==0
            || symbol.size()+1 > ASSET_TRANSFER_SYMBOL_FIX_SIZE);
}

void asset_transfer::reset()
{
    symbol = "";
    quantity = 0;
}

bool asset_transfer::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool asset_transfer::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool asset_transfer::from_data(reader& source)
{
    reset();
    symbol = source.read_string();
    quantity = source.read_8_bytes_little_endian();

    auto result = static_cast<bool>(source);
    if (!result)
        reset();

    return result;
}

data_chunk asset_transfer::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void asset_transfer::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void asset_transfer::to_data(writer& sink) const
{
    sink.write_string(symbol);
    sink.write_8_bytes_little_endian(quantity);
}

uint64_t asset_transfer::serialized_size() const
{
    size_t len = symbol.size() + 8 + 1;
    return std::min(len, ASSET_TRANSFER_FIX_SIZE);
}

std::string asset_transfer::to_string() const
{
    std::ostringstream ss;

    ss << "\t symbol = " << symbol << "\n"
        << "\t quantity = " << quantity << "\n";

    return ss.str();
}

const std::string& asset_transfer::get_symbol() const
{
    return symbol;
}
void asset_transfer::set_symbol(const std::string& symbol)
{
     size_t len = std::min(symbol.size()+1, ASSET_TRANSFER_SYMBOL_FIX_SIZE);
     this->symbol = symbol.substr(0, len);
}

uint64_t asset_transfer::get_quantity() const
{
    return quantity;
}
void asset_transfer::set_quantity(uint64_t quantity)
{
     this->quantity = quantity;
}


} // namspace chain
} // namspace libbitcoin
