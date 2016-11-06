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
#include <bitcoin/bitcoin/chain/attachment/attachment.hpp>
#include <bitcoin/bitcoin/chain/attachment/variant_visitor.hpp>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <bitcoin/bitcoin/utility/container_sink.hpp>
#include <bitcoin/bitcoin/utility/container_source.hpp>
#include <bitcoin/bitcoin/utility/istream_reader.hpp>
#include <bitcoin/bitcoin/utility/ostream_writer.hpp>

#define ASSET_TYPE static_cast<typename std::underlying_type<attachment_type>::type>(attachment_type::attachment_asset)
#define ETP_TYPE   static_cast<typename std::underlying_type<attachment_type>::type>(attachment_type::attachment_none)

namespace libbitcoin {
namespace chain {
attachment attachment::factory_from_data(const data_chunk& data)
{
    attachment instance;
    instance.from_data(data);
    return instance;
}

attachment attachment::factory_from_data(std::istream& stream)
{
    attachment instance;
    instance.from_data(stream);
    return instance;
}

attachment attachment::factory_from_data(reader& source)
{
    attachment instance;
    instance.from_data(source);
    return instance;
}

void attachment::reset()
{
	version = 0;
    type = 0; //attachment_type::attach_none;
    auto visitor = reset_visitor();
	boost::apply_visitor(visitor, attach);
}
bool attachment::is_valid() const
{
    return true;
}

bool attachment::is_valid_type() const
{
    return ((ETP_TYPE == type)
		|| (ASSET_TYPE == type));
}


bool attachment::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool attachment::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool attachment::from_data(reader& source)
{
    reset();

    version = source.read_4_bytes_little_endian();
    auto result = static_cast<bool>(source);
	
    if (result)        
		type = source.read_4_bytes_little_endian();

	result = static_cast<bool>(source);
    if (result && is_valid_type()) {
		switch(type) {
			case ETP_TYPE:
			{
				attach = etp();
				break;
			}
			case ASSET_TYPE:
			{
				attach = asset();
				break;
			}
		}
		auto visitor = from_data_visitor(source);
		result = boost::apply_visitor(visitor, attach);
    }
	else {
		result = false;
        reset();
	}

    return result;
	
}

data_chunk attachment::to_data() 
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void attachment::to_data(std::ostream& stream) 
{
    ostream_writer sink(stream);
    to_data(sink);
}

void attachment::to_data(writer& sink) 
{
    sink.write_4_bytes_little_endian(version);
    sink.write_4_bytes_little_endian(type);
	auto visitor = to_data_visitor(sink);
	boost::apply_visitor(visitor, attach);
}

uint64_t attachment::serialized_size() 
{
    uint64_t size = 4 + 4;
	auto visitor = serialized_size_visitor();
	size += boost::apply_visitor(visitor, attach);

	return size;
}

std::string attachment::to_string() 
{
    std::ostringstream ss;

	ss << "\t version = " << version << "\n"
		<< "\t type = " << type << "\n";
	auto visitor = to_string_visitor();
	ss << boost::apply_visitor(visitor, attach);

    return ss.str();
}

} // namspace chain
} // namspace libbitcoin
