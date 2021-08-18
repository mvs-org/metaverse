/**
 * Copyright (c) 2011-2021 metaverse developers (see AUTHORS)
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

void did::reset()
{
    status = 0; //did_status::did_none;
    data.reset();
}

bool did::is_valid() const
{
    return data.is_valid();
}

bool did::is_valid_type() const
{
    return ((DID_DETAIL_TYPE == status)
        || (DID_TRANSFERABLE_TYPE == status));
}

bool did::from_data_t(reader& source)
{
    reset();

    status = source.read_4_bytes_little_endian();
    auto result = static_cast<bool>(source);

    if (result && is_valid_type()) {
        result = data.from_data(source);
    }
    else {
        result = false;
        reset();
    }

    return result;
}

void did::to_data_t(writer& sink) const
{
    sink.write_4_bytes_little_endian(status);
    data.to_data(sink);
}

uint64_t did::serialized_size() const
{
    return 4 + data.serialized_size();
}

std::string did::to_string() const
{
    std::ostringstream ss;
    ss << "\t status = " << status << "\n";
    ss << data.to_string();
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

const did_detail& did::get_data() const
{
    return this->data;
}

} // namspace chain
} // namspace libbitcoin
