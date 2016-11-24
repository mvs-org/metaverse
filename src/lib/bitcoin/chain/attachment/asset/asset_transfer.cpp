/**
 * Copyright (c) 2011-2015 metaverse developers (see AUTHORS)
 *
 * This file is part of mvs-node.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
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
#include <bitcoin/bitcoin/chain/attachment/asset/asset_transfer.hpp>

#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <bitcoin/bitcoin/utility/container_sink.hpp>
#include <bitcoin/bitcoin/utility/container_source.hpp>
#include <bitcoin/bitcoin/utility/istream_reader.hpp>
#include <bitcoin/bitcoin/utility/ostream_writer.hpp>

namespace libbitcoin {
namespace chain {

asset_transfer::asset_transfer()
{
	reset();
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
    return true;
}

void asset_transfer::reset()
{	
    address = "";
    sender = "";
	recipient = "";
	status = 0;
	maximum_supply = 0;
    quantity = 0;
	timestamp = 0;
	height = 0;
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
    sender = source.read_string();
    recipient = source.read_string();
	status = source.read_4_bytes_little_endian();
    maximum_supply = source.read_8_bytes_little_endian();
    quantity = source.read_8_bytes_little_endian();
    timestamp = source.read_8_bytes_little_endian(); 
    height =  source.read_8_bytes_little_endian();
	
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
    sink.write_string(sender);
    sink.write_string(recipient);
    sink.write_4_bytes_little_endian(status);
	sink.write_8_bytes_little_endian(maximum_supply);
	sink.write_8_bytes_little_endian(quantity);
	sink.write_8_bytes_little_endian(timestamp);
	sink.write_8_bytes_little_endian(height);
}

uint64_t asset_transfer::serialized_size() const
{
    return address.size() + sender.size() + recipient.size() 
		+ 4 + 8 + 8 + 8 + 8;
}

std::string asset_transfer::to_string() 
{
    std::ostringstream ss;

    ss << "\t address = " << address << "\n"
		<< "\t sender = " << sender << "\n"
		<< "\t recipient = " << recipient << "\n"
		<< "\t status = " << status << "\n"
		<< "\t maximum_supply = " << maximum_supply << "\n"
		<< "\t quantity = " << quantity << "\n"
		<< "\t timestamp = " << timestamp << "\n"
        << "\t height=" << height << "\n";

    return ss.str();
}

} // namspace chain
} // namspace libbitcoin
