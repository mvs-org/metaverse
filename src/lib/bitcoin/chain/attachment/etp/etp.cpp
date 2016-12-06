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
#include <bitcoin/bitcoin/chain/attachment/etp/etp.hpp>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <bitcoin/bitcoin/utility/container_sink.hpp>
#include <bitcoin/bitcoin/utility/container_source.hpp>
#include <bitcoin/bitcoin/utility/istream_reader.hpp>
#include <bitcoin/bitcoin/utility/ostream_writer.hpp>

namespace libbitcoin {
namespace chain {

etp::etp()
{
	value = 0;
}
etp::etp(uint64_t value):
	value(value)
{

}

etp etp::factory_from_data(const data_chunk& data)
{
    etp instance;
    instance.from_data(data);
    return instance;
}

etp etp::factory_from_data(std::istream& stream)
{
    etp instance;
    instance.from_data(stream);
    return instance;
}

etp etp::factory_from_data(reader& source)
{
    etp instance;
    instance.from_data(source);
    return instance;
}

void etp::reset()
{
	value= 0;
}
bool etp::is_valid() const
{
    return true;
}

bool etp::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool etp::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool etp::from_data(reader& source)
{
    reset();
    value = source.read_8_bytes_little_endian();
    auto result = static_cast<bool>(source);
	
    return result;
}

data_chunk etp::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void etp::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void etp::to_data(writer& sink) const
{
	sink.write_8_bytes_little_endian(value);
}

uint64_t etp::serialized_size() const
{
    uint64_t size = 8;
	return size;
}

std::string etp::to_string() const
{
    std::ostringstream ss;
	ss << "\t value = " << value << "\n";

    return ss.str();
}
uint64_t etp::get_value() const
{
	return value;
}

void etp::set_value(uint64_t value)
{
	this->value = value;
}

} // namspace chain
} // namspace libbitcoin
