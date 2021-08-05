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
#include <metaverse/bitcoin/chain/attachment/asset/blockchain_cert.hpp>

#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>

namespace libbitcoin {
namespace chain {

blockchain_cert::blockchain_cert()
{
    reset();
}
blockchain_cert::blockchain_cert(uint32_t version, const output_point& tx_point,
            uint64_t height, const asset_cert& cert):
    version_(version), tx_point_(tx_point), height_(height), cert_(cert)
{
}

bool blockchain_cert::is_valid() const
{
    return true;
}

void blockchain_cert::reset()
{
    version_ = 0;
    tx_point_ = output_point();
    height_ = 0;
    cert_ = asset_cert();
}

bool blockchain_cert::from_data_t(reader& source)
{
    reset();

    version_ = source.read_4_bytes_little_endian();
    tx_point_.from_data(source);
    height_ = source.read_8_bytes_little_endian();
    cert_.from_data(source);

    return true;
}

void blockchain_cert::to_data_t(writer& sink) const
{
    sink.write_4_bytes_little_endian(version_);
    tx_point_.to_data(sink);
    sink.write_8_bytes_little_endian(height_);
    cert_.to_data(sink);
}

uint64_t blockchain_cert::serialized_size() const
{
    return 4 + tx_point_.serialized_size() + 8 + cert_.serialized_size();
}

#ifdef MVS_DEBUG
std::string blockchain_cert::to_string() const
{
    std::ostringstream ss;

    ss << "\t version = " << version_ << "\n"
        << "\t tx_point = " << tx_point_.to_string() << "\n"
        << "\t height = " << height_ << "\n"
        << "\t cert = " << cert_.to_string() << "\n";

    return ss.str();
}

#endif
const uint32_t& blockchain_cert::get_version() const
{
    return version_;
}
void blockchain_cert::set_version(const uint32_t& version_)
{
     this->version_ = version_;
}

const output_point& blockchain_cert::get_tx_point() const
{
    return tx_point_;
}
void blockchain_cert::set_tx_point(const output_point& tx_point_)
{
     this->tx_point_ = tx_point_;
}

const uint64_t& blockchain_cert::get_height() const
{
    return height_;
}
void blockchain_cert::set_height(const uint64_t& height_)
{
     this->height_ = height_;
}

const asset_cert& blockchain_cert::get_cert() const
{
    return cert_;
}
void blockchain_cert::set_cert(const asset_cert& cert_)
{
     this->cert_ = cert_;
}


} // namspace chain
} // namspace libbitcoin
