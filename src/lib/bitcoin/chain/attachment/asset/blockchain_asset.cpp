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
#include <metaverse/bitcoin/chain/attachment/asset/blockchain_asset.hpp>

#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>

namespace libbitcoin {
namespace chain {

blockchain_asset::blockchain_asset()
{
    reset();
}
blockchain_asset::blockchain_asset(uint32_t version, const output_point& tx_point,
            uint64_t height, const asset_detail& asset):
    version_(version), tx_point_(tx_point), height_(height), asset_(asset)
{
}

bool blockchain_asset::is_valid() const
{
    return true;
}

void blockchain_asset::reset()
{
    version_ = 0;
    tx_point_ = output_point();
    height_ = 0;
    asset_ = asset_detail();
}


bool blockchain_asset::from_data_t(reader& source)
{
    reset();

    version_ = source.read_4_bytes_little_endian();
    tx_point_.from_data(source);
    height_ = source.read_8_bytes_little_endian();
    asset_.from_data(source);

    return true;
}


void blockchain_asset::to_data_t(writer& sink) const
{
    sink.write_4_bytes_little_endian(version_);
    tx_point_.to_data(sink);
    sink.write_8_bytes_little_endian(height_);
    asset_.to_data(sink);
}

uint64_t blockchain_asset::serialized_size() const
{
    return 4 + tx_point_.serialized_size() + 8 + asset_.serialized_size();
}

#ifdef MVS_DEBUG
std::string blockchain_asset::to_string() const
{
    std::ostringstream ss;

    ss << "\t version = " << version_ << "\n"
        << "\t tx_point = " << tx_point_.to_string() << "\n"
        << "\t height = " << height_ << "\n"
        << "\t asset = " << asset_.to_string() << "\n";

    return ss.str();
}

#endif
const uint32_t& blockchain_asset::get_version() const
{
    return version_;
}
void blockchain_asset::set_version(const uint32_t& version_)
{
     this->version_ = version_;
}

const output_point& blockchain_asset::get_tx_point() const
{
    return tx_point_;
}
void blockchain_asset::set_tx_point(const output_point& tx_point_)
{
     this->tx_point_ = tx_point_;
}

const uint64_t& blockchain_asset::get_height() const
{
    return height_;
}
void blockchain_asset::set_height(const uint64_t& height_)
{
     this->height_ = height_;
}

const asset_detail& blockchain_asset::get_asset() const
{
    return asset_;
}
void blockchain_asset::set_asset(const asset_detail& asset_)
{
     this->asset_ = asset_;
}



} // namspace chain
} // namspace libbitcoin
