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
#include <metaverse/bitcoin/chain/attachment/asset/asset.hpp>
#include <metaverse/bitcoin/chain/attachment/variant_visitor.hpp>
#include <metaverse/bitcoin/chain/attachment/asset/asset_detail.hpp>
#include <metaverse/bitcoin/chain/attachment/asset/asset_transfer.hpp>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>

namespace libbitcoin {
namespace chain {

asset::asset()
{
    reset();
}
asset::asset(uint32_t status, const asset_detail& detail):
    status(status), data(detail)
{
}
asset::asset(uint32_t status, const asset_transfer& detail):
    status(status), data(detail)
{
}
asset asset::factory_from_data(const data_chunk& data)
{
    asset instance;
    instance.from_data(data);
    return instance;
}

asset asset::factory_from_data(std::istream& stream)
{
    asset instance;
    instance.from_data(stream);
    return instance;
}

asset asset::factory_from_data(reader& source)
{
    asset instance;
    instance.from_data(source);
    return instance;
}

void asset::reset()
{
    status = 0; //asset_status::asset_none;
    auto visitor = reset_visitor();
    boost::apply_visitor(visitor, data);
}

bool asset::is_valid() const
{
    return true;
}

bool asset::is_valid_type() const
{
    return ((ASSET_DETAIL_TYPE == status)
        || (ASSET_TRANSFERABLE_TYPE == status));
}

bool asset::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool asset::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool asset::from_data(reader& source)
{
    reset();

    status = source.read_4_bytes_little_endian();
    auto result = static_cast<bool>(source);

    if (result && is_valid_type()) {
        switch(status) {
            case ASSET_DETAIL_TYPE:
            {
                data = asset_detail();
                break;
            }
            case ASSET_TRANSFERABLE_TYPE:
            {
                data = asset_transfer();
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

data_chunk asset::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void asset::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void asset::to_data(writer& sink) const
{
    sink.write_4_bytes_little_endian(status);

    auto visitor = to_data_visitor(sink);
    boost::apply_visitor(visitor, data);
}

uint64_t asset::serialized_size() const
{
    uint64_t size = 0;

    auto visitor = serialized_size_visitor();
    size += boost::apply_visitor(visitor, data);
    return 4 + size;
}

std::string asset::to_string() const
{
    std::ostringstream ss;
    ss << "\t status = " << status << "\n";
    auto visitor = to_string_visitor();
    ss << boost::apply_visitor(visitor, data);
    return ss.str();
}

uint32_t asset::get_status() const
{
    return status;
}
void asset::set_status(uint32_t status)
{
    this->status = status;
}
void asset::set_data(const asset_detail& detail)
{
    this->data = detail;
}
void asset::set_data(const asset_transfer& detail)
{
    this->data = detail;
}
asset::asset_data_type& asset::get_data()
{
    return this->data;
}
const asset::asset_data_type& asset::get_data() const
{
    return this->data;
}

} // namspace chain
} // namspace libbitcoin
