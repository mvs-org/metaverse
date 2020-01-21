/**
 * Copyright (c) 2019-2020 metaverse developers (see AUTHORS)
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
#include <metaverse/bitcoin/chain/attachment/etp/etp_award.hpp>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>

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

void etp_award::reset()
{
    height= 0;
}
bool etp_award::is_valid() const
{
    return true;
}

bool etp_award::from_data_t(reader& source)
{
    reset();
    height = source.read_8_bytes_little_endian();
    auto result = static_cast<bool>(source);

    return result;
}


void etp_award::to_data_t(writer& sink) const
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
