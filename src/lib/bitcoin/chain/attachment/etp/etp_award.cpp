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
#include <metaverse/lib/bitcoin/chain/attachment/etp/etp_award.hpp>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <metaverse/lib/bitcoin/utility/container_sink.hpp>
#include <metaverse/lib/bitcoin/utility/container_source.hpp>
#include <metaverse/lib/bitcoin/utility/istream_reader.hpp>
#include <metaverse/lib/bitcoin/utility/ostream_writer.hpp>

namespace libbitcoin {
namespace chain {

etp_award::etp_award()
{
	height = 0;
}
etp_award::etp_award(uint64_t height):
	height(height)
{

}

etp_award etp_award::factory_from_data(const data_chunk& data)
{
    etp_award instance;
    instance.from_data(data);
    return instance;
}

etp_award etp_award::factory_from_data(std::istream& stream)
{
    etp_award instance;
    instance.from_data(stream);
    return instance;
}

etp_award etp_award::factory_from_data(reader& source)
{
    etp_award instance;
    instance.from_data(source);
    return instance;
}

void etp_award::reset()
{
	height= 0;
}
bool etp_award::is_valid() const
{
    return true;
}

bool etp_award::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool etp_award::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool etp_award::from_data(reader& source)
{
    reset();
    height = source.read_8_bytes_little_endian();
    auto result = static_cast<bool>(source);
	
    return result;
}

data_chunk etp_award::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void etp_award::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void etp_award::to_data(writer& sink) const
{
	sink.write_8_bytes_little_endian(height);
}

uint64_t etp_award::serialized_size() const
{
    //uint64_t size = 8;
	return 8;
}

std::string etp_award::to_string() const
{
    std::ostringstream ss;
	ss << "\t height = " << height << "\n";

    return ss.str();
}
uint64_t etp_award::get_height() const
{
	return height;
}

void etp_award::set_height(uint64_t height)
{
	this->height = height;
}

} // namspace chain
} // namspace libbitcoin
