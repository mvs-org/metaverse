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
#include <metaverse/bitcoin/chain/attachment/did/did.hpp>
#include <metaverse/bitcoin/chain/attachment/variant_visitor.hpp>
#include <metaverse/bitcoin/chain/attachment/did/did_detail.hpp>
#include <metaverse/bitcoin/chain/attachment/did/did_transfer.hpp>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>

namespace libbitcoin {
namespace chain {

did::did()
{
	reset();
}
did::did(uint32_t status, const did_detail& detail):
	status(status), data(detail)
{
}
did::did(uint32_t status, const did_transfer& detail):
	status(status), data(detail)
{
}
did did::factory_from_data(const data_chunk& data)
{
    did instance;
    instance.from_data(data);
    return instance;
}

did did::factory_from_data(std::istream& stream)
{
    did instance;
    instance.from_data(stream);
    return instance;
}

did did::factory_from_data(reader& source)
{
    did instance;
    instance.from_data(source);
    return instance;
}

void did::reset()
{
    status = 0; //did_status::did_none;
    auto visitor = reset_visitor();
	boost::apply_visitor(visitor, data);
}

bool did::is_valid() const
{
    return true;
}

bool did::is_valid_type() const
{
    return ((DID_DETAIL_TYPE == status)
		|| (DID_TRANSFERABLE_TYPE == status));
}

bool did::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool did::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool did::from_data(reader& source)
{
    reset();

    status = source.read_4_bytes_little_endian();
    auto result = static_cast<bool>(source);
	
    if (result && is_valid_type()) {
		switch(status) {
			case DID_DETAIL_TYPE:
			{
				data = did_detail();
				break;
			}
			case DID_TRANSFERABLE_TYPE:
			{
				data = did_transfer();
				break;
			}			
		}	
		auto visitor = from_data_visitor(source);
		result = boost::apply_visitor(visitor, data);
    }
	else {
		result = false;
		reset();
	}
		
    return result;
}

data_chunk did::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void did::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void did::to_data(writer& sink) const
{
    sink.write_4_bytes_little_endian(status);
	
    auto visitor = to_data_visitor(sink);
	boost::apply_visitor(visitor, data);
}

uint64_t did::serialized_size() const
{
    uint64_t size = 0;
	
    auto visitor = serialized_size_visitor();
	size += boost::apply_visitor(visitor, data);
	return 4 + size;
}

std::string did::to_string() const
{
    std::ostringstream ss;
    ss << "\t status = " << status << "\n";
    auto visitor = to_string_visitor();
	ss << boost::apply_visitor(visitor, data);
    return ss.str();
}

uint32_t did::get_status() const
{
	return status;
}
void did::set_status(uint32_t status)
{
	this->status = status;
}
void did::set_data(const did_detail& detail)
{
	this->data = detail;
}
void did::set_data(const did_transfer& detail)
{
	this->data = detail;
}
did::did_data_type& did::get_data()
{
	return this->data;
}

} // namspace chain
} // namspace libbitcoin
