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
#include <bitcoin/bitcoin/chain/attachment/asset/asset_detail.hpp>

#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <bitcoin/bitcoin/utility/container_sink.hpp>
#include <bitcoin/bitcoin/utility/container_source.hpp>
#include <bitcoin/bitcoin/utility/istream_reader.hpp>
#include <bitcoin/bitcoin/utility/ostream_writer.hpp>
#include <json/minijson_writer.hpp>

namespace libbitcoin {
namespace chain {

asset_detail::asset_detail()
{
	reset();
}
asset_detail::asset_detail(
    std::string symbol, uint64_t maximum_supply,
    uint32_t asset_type, std::string issuer,
    std::string address, std::string description)
{
    this->symbol = symbol;
    this->maximum_supply = maximum_supply;
    this->asset_type = asset_type;
    this->issuer = issuer; 
    this->address = address;
    this->description = description;
}

asset_detail asset_detail::factory_from_data(const data_chunk& data)
{
    asset_detail instance;
    instance.from_data(data);
    return instance;
}

asset_detail asset_detail::factory_from_data(std::istream& stream)
{
    asset_detail instance;
    instance.from_data(stream);
    return instance;
}

asset_detail asset_detail::factory_from_data(reader& source)
{
    asset_detail instance;
    instance.from_data(source);
    return instance;
}

bool asset_detail::is_valid() const
{
    return !(symbol.empty() || (maximum_supply==0));
}

void asset_detail::reset()
{	
    symbol = "";
    maximum_supply = 0;
    asset_type = 0;
    issuer = ""; 
    address = "";
    description = "";
    //issue_price = 0;

    //restrict section
    //number_of_decimal_point = 0; //number of decimal point
    //life circle
    //flag = 0; //is_white_list/is_tx_backwards/is_require_approval
    
    // relationship section
    //fee = 0.0;
    //correlation asset
    //authentication_organization = ""; //authentication organization
}

bool asset_detail::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool asset_detail::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool asset_detail::from_data(reader& source)
{
    reset();

    symbol = source.read_string();
    maximum_supply = source.read_8_bytes_little_endian();
    asset_type = source.read_4_bytes_little_endian();
    issuer = source.read_string(); 
    address =  source.read_string();
    description =  source.read_string();
    //issue_price =  source.read_8_bytes_little_endian();

    //restrict section
    //number_of_decimal_point =  source.read_4_bytes_little_endian(); //number of decimal point
    //life circle
    //flag =  source.read_8_bytes_little_endian(); //is_white_list/is_tx_backwards/is_require_approval
    
    // relationship section
    //double fee;
    //correlation asset
    //authentication_organization =  source.read_string(); //authentication organization
    auto result = static_cast<bool>(source);
    if (!result)
        reset();

    return result;	
}

data_chunk asset_detail::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
	std::cout<<"to-data: "<<data.size()<<" "<<serialized_size();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void asset_detail::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void asset_detail::to_data(writer& sink) const
{
    sink.write_string(symbol);
    sink.write_8_bytes_little_endian(maximum_supply);
	sink.write_4_bytes_little_endian(asset_type);
	sink.write_string(issuer);
	sink.write_string(address);
	sink.write_string(description);
}

uint64_t asset_detail::serialized_size() const
{
    return symbol.size() + 8 + 4 + issuer.size() + address.size() + description.size();
}

std::string asset_detail::to_string() 
{
    std::ostringstream ss;

    ss << "\t symbol = " << symbol << "\n"
		<< "\t maximum_supply = " << maximum_supply << "\n"
		<< "\t asset_type = " << asset_type << "\n"
		<< "\t issuer = " << issuer << "\n"
		<< "\t address = " << address << "\n"
        << "\t description=" << description << "\n";

    return ss.str();
}

void asset_detail::to_json(std::ostream& output) 
{
	minijson::object_writer json_writer(output);
	json_writer.write("symbol", symbol);
	json_writer.write("maximum_supply", maximum_supply);
	json_writer.write("asset_type", asset_type);
	json_writer.write("issuer", issuer);
	json_writer.write("address", address);
	json_writer.write("description", description);
	json_writer.close();
}

} // namspace chain
} // namspace libbitcoin
