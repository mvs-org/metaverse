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
#include <metaverse/bitcoin/chain/history.hpp>
#include <json/minijson_writer.hpp>

namespace libbitcoin {
namespace chain {

bool asset_balances::operator< (const asset_balances& other) const
{
    typedef std::tuple<std::string, std::string, uint64_t, uint64_t> cmp_tuple;
    return cmp_tuple(symbol, address, unspent_asset, locked_asset)
        < cmp_tuple(other.symbol, other.address, other.unspent_asset, other.locked_asset);
}

asset_transfer::asset_transfer()
{
    reset();
}
asset_transfer::asset_transfer(const std::string& symbol, uint64_t quantity):
    symbol(symbol),quantity(quantity)
{

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

bool asset_transfer::from_data_t(reader& source)
{
    reset();
    symbol = source.read_string();
    quantity = source.read_8_bytes_little_endian();

    auto result = static_cast<bool>(source);
    if (!result)
        reset();

    return result;
}


void asset_transfer::to_data_t(writer& sink) const
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
