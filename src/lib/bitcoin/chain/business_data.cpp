/**
 * Copyright (c) 2011-2020 metaverse developers (see AUTHORS)
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
#include <metaverse/bitcoin/chain/business_data.hpp>
#include <metaverse/bitcoin/chain/attachment/variant_visitor.hpp>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>

#define TYPE_ETP            KIND2UINT16(business_kind::etp)
#define TYPE_ETP_AWARD      KIND2UINT16(business_kind::etp_award)
#define TYPE_ASSET_ISSUE    KIND2UINT16(business_kind::asset_issue)
#define TYPE_ASSET_TRANSFER KIND2UINT16(business_kind::asset_transfer)
#define TYPE_ASSET_CERT     KIND2UINT16(business_kind::asset_cert)
#define TYPE_ASSET_MIT      KIND2UINT16(business_kind::asset_mit)
#define TYPE_MESSAGE        KIND2UINT16(business_kind::message)
#define TYPE_DID_REGISTER   KIND2UINT16(business_kind::did_register)
#define TYPE_DID_TRANSFER   KIND2UINT16(business_kind::did_transfer)

namespace libbitcoin {
namespace chain {

void business_data::reset()
{
    kind = business_kind::etp;
    timestamp = 0;
    auto visitor = reset_visitor();
    boost::apply_visitor(visitor, data);
}
bool business_data::is_valid() const
{
    return true;
}

bool business_data::is_valid_type() const
{
    return ((TYPE_ETP == KIND2UINT16(kind))
            || (TYPE_ASSET_ISSUE == KIND2UINT16(kind))
            || (TYPE_ASSET_TRANSFER == KIND2UINT16(kind))
            || (TYPE_ASSET_CERT == KIND2UINT16(kind))
            || (TYPE_ASSET_MIT == KIND2UINT16(kind)))
            || (TYPE_ETP_AWARD == KIND2UINT16(kind))
            || (TYPE_MESSAGE == KIND2UINT16(kind))
            || (TYPE_DID_REGISTER == KIND2UINT16(kind))
            || (TYPE_DID_TRANSFER == KIND2UINT16(kind));
}

bool business_data::from_data_t(reader& source)
{
    reset();
    kind = static_cast<business_kind>(source.read_2_bytes_little_endian());
    timestamp = source.read_4_bytes_little_endian();
    auto result = static_cast<bool>(source);

    if (result && is_valid_type())
    {
        switch (KIND2UINT16(kind))
        {
            case TYPE_ETP:
            {
                data = etp();
                break;
            }
            case TYPE_ETP_AWARD:
            {
                data = etp_award();
                break;
            }
            case TYPE_ASSET_ISSUE:
            {
                data = asset_detail();
                break;
            }
            case TYPE_ASSET_TRANSFER:
            {
                data = asset_transfer();
                break;
            }
            case TYPE_ASSET_CERT:
            {
                data = asset_cert();
                break;
            }
            case TYPE_ASSET_MIT:
            {
                data = asset_mit();
                break;
            }
            case TYPE_MESSAGE:
            {
                data = blockchain_message();
                break;
            }
            case TYPE_DID_REGISTER:
            {
                data = did_detail();
                break;
            }
            case TYPE_DID_TRANSFER:
            {
                data = did_detail();
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

void business_data::to_data_t(writer& sink)
{
    sink.write_2_bytes_little_endian(KIND2UINT16(kind));
    sink.write_4_bytes_little_endian(timestamp);
    auto visitor = to_data_visitor(sink);
    boost::apply_visitor(visitor, data);
}

uint64_t business_data::serialized_size()
{
    uint64_t size = 2 + 4; // kind and timestamp
    auto visitor = serialized_size_visitor();
    size += boost::apply_visitor(visitor, data);

    return size;
}

#ifdef MVS_DEBUG
std::string business_data::to_string()
{
    std::ostringstream ss;

    ss << "\t kind = " << KIND2UINT16(kind) << "\n";
    ss << "\t timestamp = " << timestamp << "\n";
    auto visitor = to_string_visitor();
    ss << boost::apply_visitor(visitor, data);

    return ss.str();
}
#endif

business_kind business_data::get_kind_value() const
{
    //return KIND2UINT16(kind);
    return kind;
}

const business_data::business_data_type& business_data::get_data() const
{
    return data;
}

uint32_t business_data::get_timestamp() const
{
    return timestamp;
}

} // namspace chain
} // namspace libbitcoin
