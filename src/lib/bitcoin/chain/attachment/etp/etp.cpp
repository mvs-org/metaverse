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
#include <metaverse/bitcoin/chain/attachment/etp/etp.hpp>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>

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

void etp::reset()
{
    value= 0;
}
bool etp::is_valid() const
{
    return true;
}


bool etp::from_data_t(reader& source)
{
    /*
    reset();
    value = source.read_8_bytes_little_endian();
    auto result = static_cast<bool>(source);

    return result;
    */
    return true;
}

void etp::to_data_t(writer& sink) const
{
    //sink.write_8_bytes_little_endian(value); // not use etp now
}

uint64_t etp::serialized_size() const
{
    //uint64_t size = 8;
    //return size;
    return 0; // not insert ept into transaction
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
