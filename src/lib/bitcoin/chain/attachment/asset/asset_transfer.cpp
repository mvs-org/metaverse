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
#ifdef MVS_DEBUG
#include <json/minijson_writer.hpp>
#endif

namespace libbitcoin {
namespace chain {

asset_transfer::asset_transfer()
{
	reset();
}
asset_transfer::asset_transfer(const std::string& address, uint64_t quantity):
	address(address),quantity(quantity)
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
    return !(address.empty() 
			|| quantity==0
			|| address.size() +1 > ASSET_TRANSFER_ADDRESS_FIX_SIZE);
}

void asset_transfer::reset()
{	
    address = "";
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
    address = source.read_string();
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
    sink.write_string(address);
	sink.write_8_bytes_little_endian(quantity);
}

uint64_t asset_transfer::serialized_size() const
{
    size_t len = address.size() + 8 + 1;
	return std::min(len, ASSET_TRANSFER_FIX_SIZE);
}

#ifdef MVS_DEBUG
std::string asset_transfer::to_string() const 
{
    std::ostringstream ss;

    ss << "\t address = " << address << "\n"
		<< "\t quantity = " << quantity << "\n";

    return ss.str();
}

void asset_transfer::to_json(std::ostream& output) 
{
	minijson::object_writer json_writer(output);
	json_writer.write("address", address);
	json_writer.write("quantity", quantity);
	json_writer.close();
}
#endif

const std::string& asset_transfer::get_address() const
{ 
    return address;
}
void asset_transfer::set_address(const std::string& address)
{ 
	 size_t len = address.size()+1 < (ASSET_TRANSFER_ADDRESS_FIX_SIZE) ?address.size()+1:ASSET_TRANSFER_ADDRESS_FIX_SIZE;
	 this->address = address.substr(0, len);
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
