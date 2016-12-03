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
#include <bitcoin/bitcoin/chain/business_data.hpp>
#include <bitcoin/bitcoin/chain/attachment/variant_visitor.hpp>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <bitcoin/bitcoin/utility/container_sink.hpp>
#include <bitcoin/bitcoin/utility/container_source.hpp>
#include <bitcoin/bitcoin/utility/istream_reader.hpp>
#include <bitcoin/bitcoin/utility/ostream_writer.hpp>

#define KIND2UINT16(kd)  (static_cast<typename std::underlying_type<business_kind>::type>(kd))
#define ETP_TYPE            KIND2UINT16(business_kind::etp)
#define ASSET_ISSUE_TYPE    KIND2UINT16(business_kind::asset_issue)
#define ASSET_TRANSFER_TYPE KIND2UINT16(business_kind::asset_transfer)

namespace libbitcoin {
namespace chain {
	
business_data business_data::factory_from_data(const data_chunk& data)
{
    business_data instance;
    instance.from_data(data);
    return instance;
}

business_data business_data::factory_from_data(std::istream& stream)
{
    business_data instance;
    instance.from_data(stream);
    return instance;
}

business_data business_data::factory_from_data(reader& source)
{
    business_data instance;
    instance.from_data(source);
    return instance;
}

void business_data::reset()
{
	kind = business_kind::etp;
    auto visitor = reset_visitor();
	boost::apply_visitor(visitor, data);
}
bool business_data::is_valid() const
{
    return true;
}

bool business_data::is_valid_type() const
{
    return ((ETP_TYPE == KIND2UINT16(kind))
		|| (ASSET_ISSUE_TYPE == KIND2UINT16(kind))
		|| (ASSET_TRANSFER_TYPE == KIND2UINT16(kind)));
}


bool business_data::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool business_data::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool business_data::from_data(reader& source)
{
    reset();
    kind = static_cast<business_kind>(source.read_2_bytes_little_endian());
    auto result = static_cast<bool>(source);
	
    if (result && is_valid_type()) 
	{
		switch(KIND2UINT16(kind)) 
		{
			case ETP_TYPE:
			{
				data = etp();
				break;
			}
			case ASSET_ISSUE_TYPE:
			{
				data = asset_detail();
				break;
			}
			case ASSET_TRANSFER_TYPE:
			{
				data = asset_transfer();
				break;
			}
		}
		auto visitor = from_data_visitor(source);
		result = boost::apply_visitor(visitor, data);
    }
	else 
	{
		result = false;
        reset();
	}

    return result;
	
}

data_chunk business_data::to_data() 
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void business_data::to_data(std::ostream& stream) 
{
    ostream_writer sink(stream);
    to_data(sink);
}

void business_data::to_data(writer& sink) 
{
    sink.write_2_bytes_little_endian(KIND2UINT16(kind));
	auto visitor = to_data_visitor(sink);
	boost::apply_visitor(visitor, data);
}

uint64_t business_data::serialized_size() 
{
    uint64_t size = 4;
	auto visitor = serialized_size_visitor();
	size += boost::apply_visitor(visitor, data);

	return size;
}

std::string business_data::to_string() 
{
    std::ostringstream ss;

	ss << "\t kind = " << KIND2UINT16(kind) << "\n";
	auto visitor = to_string_visitor();
	ss << boost::apply_visitor(visitor, data);

    return ss.str();
}
uint16_t business_data::get_kind_value() const
{
	return KIND2UINT16(kind);
}
const business_data::business_data_type& business_data::get_data() const
{
	return data;
}


} // namspace chain
} // namspace libbitcoin
