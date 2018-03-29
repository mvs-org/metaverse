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
 * GNU Affero General Public License for more transfers.
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
did_transfer::did_transfer(
    std::string symbol, std::string issuer,
    std::string address, std::string description):
    symbol(symbol), issuer(issuer), address(address), description(description)
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
    return !(symbol.empty() 
			|| (symbol.size() + issuer.size() + address.size() + description.size() + 4)>DID_TRANSFER_FIX_SIZE);
}

void did_transfer::reset()
{	
    symbol = "";
    //did_type = 0;
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
    //correlation did
    //authentication_organization = ""; //authentication organization
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

    symbol = source.read_string();
    //did_type = source.read_4_bytes_little_endian();
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
    //correlation did
    //authentication_organization =  source.read_string(); //authentication organization
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
    sink.write_string(symbol);
	//sink.write_4_bytes_little_endian(did_type);
	sink.write_string(issuer);
	sink.write_string(address);
	sink.write_string(description);
}

uint64_t did_transfer::serialized_size() const
{
    size_t len = symbol.size()  + issuer.size() + address.size() + description.size() + 4 ;
    return std::min(DID_TRANSFER_FIX_SIZE, len);
}

std::string did_transfer::to_string() const
{
    std::ostringstream ss;

    ss << "\t symbol = " << symbol << "\n"
		<< "\t issuer = " << issuer << "\n"
		<< "\t address = " << address << "\n"
        << "\t description=" << description << "\n";

    return ss.str();
}

void did_transfer::to_json(std::ostream& output) 
{
	minijson::object_writer json_writer(output);
	json_writer.write("symbol", symbol);
	//json_writer.write("did_type", did_type);
	json_writer.write("issuer", issuer);
	json_writer.write("address", address);
	json_writer.write("description", description);
	json_writer.close();
}

const std::string& did_transfer::get_symbol() const
{ 
    return symbol;
}
void did_transfer::set_symbol(const std::string& symbol)
{ 
	size_t len = symbol.size()+1 < (DID_TRANSFER_SYMBOL_FIX_SIZE) ?symbol.size()+1:DID_TRANSFER_SYMBOL_FIX_SIZE;
    this->symbol = symbol.substr(0, len);
}

const std::string& did_transfer::get_issuer() const
{ 
    return issuer;
}
void did_transfer::set_issuer(const std::string& issuer)
{ 
	 size_t len = issuer.size()+1 < (DID_TRANSFER_ISSUER_FIX_SIZE) ?issuer.size()+1:DID_TRANSFER_ISSUER_FIX_SIZE;
	 this->issuer = issuer.substr(0, len);
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

const std::string& did_transfer::get_description() const
{ 
    return description;
}
void did_transfer::set_description(const std::string& description)
{ 
	 size_t len = description.size()+1 < (DID_TRANSFER_DESCRIPTION_FIX_SIZE) ?description.size()+1:DID_TRANSFER_DESCRIPTION_FIX_SIZE;
	 this->description = description.substr(0, len);
}


} // namspace chain
} // namspace libbitcoin
