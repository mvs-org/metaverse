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
#include <metaverse/bitcoin/chain/attachment/did/did_transfer.hpp>

#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>
#include <json/minijson_writer.hpp>

namespace libbitcoin {
namespace chain {

did_transfer::did_transfer()
{
	reset();
}
did_transfer::did_transfer(const std::string& address, uint64_t quantity):
	address(address),quantity(quantity)
{

}
did_transfer did_transfer::factory_from_data(const data_chunk& data)
{
    did_transfer instance;
    instance.from_data(data);
    return instance;
}

did_transfer did_transfer::factory_from_data(std::istream& stream)
{
    did_transfer instance;
    instance.from_data(stream);
    return instance;
}

did_transfer did_transfer::factory_from_data(reader& source)
{
    did_transfer instance;
    instance.from_data(source);
    return instance;
}

bool did_transfer::is_valid() const
{
    return !(address.empty() 
			|| quantity==0
			|| address.size() +1 > DID_TRANSFER_ADDRESS_FIX_SIZE);
}

void did_transfer::reset()
{	
    address = "";
    quantity = 0;
}

bool did_transfer::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool did_transfer::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool did_transfer::from_data(reader& source)
{
    reset();
    address = source.read_string();
    quantity = source.read_8_bytes_little_endian();
	
    auto result = static_cast<bool>(source);
    if (!result)
        reset();

    return result;	
}

data_chunk did_transfer::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void did_transfer::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void did_transfer::to_data(writer& sink) const
{
    sink.write_string(address);
	sink.write_8_bytes_little_endian(quantity);
}

uint64_t did_transfer::serialized_size() const
{
    size_t len = address.size() + 8 + 1;
	return std::min(len, DID_TRANSFER_FIX_SIZE);
}

std::string did_transfer::to_string() const 
{
    std::ostringstream ss;

    ss << "\t address = " << address << "\n"
		<< "\t quantity = " << quantity << "\n";

    return ss.str();
}

void did_transfer::to_json(std::ostream& output) 
{
	minijson::object_writer json_writer(output);
	json_writer.write("address", address);
	json_writer.write("quantity", quantity);
	json_writer.close();
}

const std::string& did_transfer::get_address() const
{ 
    return address;
}
void did_transfer::set_address(const std::string& address)
{ 
	 size_t len = address.size()+1 < (DID_TRANSFER_ADDRESS_FIX_SIZE) ?address.size()+1:DID_TRANSFER_ADDRESS_FIX_SIZE;
	 this->address = address.substr(0, len);
}

uint64_t did_transfer::get_quantity() const
{ 
    return quantity;
}
void did_transfer::set_quantity(uint64_t quantity)
{ 
     this->quantity = quantity;
}


} // namspace chain
} // namspace libbitcoin
