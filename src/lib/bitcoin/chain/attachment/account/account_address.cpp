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
#include <bitcoin/bitcoin/chain/attachment/account/account_address.hpp>

#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <bitcoin/bitcoin/utility/container_sink.hpp>
#include <bitcoin/bitcoin/utility/container_source.hpp>
#include <bitcoin/bitcoin/utility/istream_reader.hpp>
#include <bitcoin/bitcoin/utility/ostream_writer.hpp>
#include <json/minijson_writer.hpp>

namespace libbitcoin {
namespace chain {

account_address::account_address()
{
	reset();
}

account_address::account_address(std::string name, std::string xprv_key, 
	std::string xpub_key, uint32_t hd_index)
{
    this->name = name;
    this->xprv_key = xprv_key;
    this->xpub_key = xpub_key;
    this->hd_index = hd_index;
}
account_address account_address::factory_from_data(const data_chunk& data)
{
    account_address instance;
    instance.from_data(data);
    return instance;
}

account_address account_address::factory_from_data(std::istream& stream)
{
    account_address instance;
    instance.from_data(stream);
    return instance;
}

account_address account_address::factory_from_data(reader& source)
{
    account_address instance;
    instance.from_data(source);
    return instance;
}

bool account_address::is_valid() const
{
    return true;
}

void account_address::reset()
{	
    this->name = "";
    this->xprv_key = "";
    this->xpub_key = "";
    this->hd_index = 0;
}

bool account_address::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool account_address::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool account_address::from_data(reader& source)
{
    reset();
    name = source.read_string();
    xprv_key = source.read_string();
	xpub_key = source.read_string();
    hd_index= source.read_4_bytes_little_endian();
    return true;	
}

data_chunk account_address::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
	std::cout<<"to-data: "<<data.size()<<" "<<serialized_size();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void account_address::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void account_address::to_data(writer& sink) const
{
    sink.write_string(name);
	sink.write_string(xprv_key);
	sink.write_string(xpub_key);
	sink.write_4_bytes_little_endian(hd_index);
}

uint64_t account_address::serialized_size() const
{
    return name.size() + xprv_key.size() + xpub_key.size() + 4 + 3; // 3 "string length" byte
}

std::string account_address::to_string() 
{
    std::ostringstream ss;

    ss << "\t name = " << name << "\n"
		<< "\t xprv_key = " << xprv_key << "\n"
		<< "\t xpub_key = " << xpub_key << "\n"
		<< "\t hd_index = " << hd_index << "\n";

    return ss.str();
}
void account_address::to_json(std::ostream& output) 
{
	minijson::object_writer json_writer(output);
	json_writer.write("name", name);
	json_writer.write("xprv_key", xprv_key);
	json_writer.write("xpub_key", xpub_key);
	json_writer.write("hd_index", hd_index);
	json_writer.close();
}


} // namspace chain
} // namspace libbitcoin
