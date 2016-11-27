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
#include <bitcoin/bitcoin/chain/attachment/account/account.hpp>

#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <bitcoin/bitcoin/utility/container_sink.hpp>
#include <bitcoin/bitcoin/utility/container_source.hpp>
#include <bitcoin/bitcoin/utility/istream_reader.hpp>
#include <bitcoin/bitcoin/utility/ostream_writer.hpp>
#include <json/minijson_writer.hpp>

namespace libbitcoin {
namespace chain {

account::account()
{
	reset();
}
account::account(std::string name, std::string mnemonic, hash_digest passwd, 
		uint32_t hd_index, uint8_t priority)
{
    this->name = name;
    this->mnemonic = mnemonic;
    this->passwd = passwd;
    this->hd_index = hd_index;
    this->priority = priority;
}

account account::factory_from_data(const data_chunk& data)
{
    account instance;
    instance.from_data(data);
    return instance;
}

account account::factory_from_data(std::istream& stream)
{
    account instance;
    instance.from_data(stream);
    return instance;
}

account account::factory_from_data(reader& source)
{
    account instance;
    instance.from_data(source);
    return instance;
}

bool account::is_valid() const
{
    return true;
}

void account::reset()
{	
    this->name = "";
    this->mnemonic = "";
    //this->passwd = "";
    this->hd_index = 0;
    this->priority = 1; // 0 -- admin user  1 -- common user
}

bool account::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool account::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool account::from_data(reader& source)
{
    reset();
    name = source.read_string();
    mnemonic = source.read_string();
    passwd = source.read_hash();
    hd_index= source.read_4_bytes_little_endian();
    priority= source.read_byte();
    return true;	
}

data_chunk account::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
	std::cout<<"to-data: "<<data.size()<<" "<<serialized_size();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void account::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void account::to_data(writer& sink) const
{
    sink.write_string(name);
	sink.write_string(mnemonic);
	sink.write_hash(passwd);
	sink.write_4_bytes_little_endian(hd_index);
	sink.write_byte(priority);
}

uint64_t account::serialized_size() const
{
    return name.size() + mnemonic.size() + passwd.size() + 4 + 1 + 3; // 2 "string length" byte
}

std::string account::to_string() 
{
    std::ostringstream ss;

    ss << "\t name = " << name << "\n"
		<< "\t mnemonic = " << mnemonic << "\n"
		<< "\t password = " << passwd.data() << "\n"
		<< "\t hd_index = " << hd_index << "\n"
		<< "\t priority = " << priority << "\n";

    return ss.str();
}

account::operator bool() const
{
	return (name.empty() || mnemonic.empty());
}

void account::to_json(std::ostream& output) 
{
	minijson::object_writer json_writer(output);
	json_writer.write("name", name);
	json_writer.write("mnemonic", mnemonic);
	//json_writer.write_array("passwd", std::begin(passwd.data()), std::passwd.end());
	json_writer.write("hd_index", hd_index);
	json_writer.write("priority", priority);
	json_writer.close();
}

} // namspace chain
} // namspace libbitcoin
