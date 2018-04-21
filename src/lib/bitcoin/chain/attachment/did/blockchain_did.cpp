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
#include <metaverse/bitcoin/chain/attachment/did/blockchain_did.hpp>

#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>

namespace libbitcoin {
namespace chain {

blockchain_did::blockchain_did()
{
	reset();
}
blockchain_did::blockchain_did(uint32_t version, const output_point& tx_point,
			uint64_t height, uint32_t status, const did_detail& did):
    version_(version), tx_point_(tx_point), height_(height), status_(status), did_(did)
{
}

blockchain_did blockchain_did::factory_from_data(const data_chunk& data)
{
    blockchain_did instance;
    instance.from_data(data);
    return instance;
}

blockchain_did blockchain_did::factory_from_data(std::istream& stream)
{
    blockchain_did instance;
    instance.from_data(stream);
    return instance;
}

blockchain_did blockchain_did::factory_from_data(reader& source)
{
    blockchain_did instance;
    instance.from_data(source);
    return instance;
}
bool blockchain_did::is_valid() const
{
    return true;
}

void blockchain_did::reset()
{	
    version_ = 0;
    tx_point_ = output_point();
	height_ = 0;
    status_ = address_invalid;
	did_ = did_detail();
}

bool blockchain_did::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool blockchain_did::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool blockchain_did::from_data(reader& source)
{
    reset();

    version_ = source.read_4_bytes_little_endian();
	tx_point_.from_data(source);
    height_ = source.read_8_bytes_little_endian();
    status_ = source.read_4_bytes_little_endian();
	did_.from_data(source);

    return true;	
}

data_chunk blockchain_did::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    //BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void blockchain_did::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void blockchain_did::to_data(writer& sink) const
{
    sink.write_4_bytes_little_endian(version_);
	tx_point_.to_data(sink);
    sink.write_8_bytes_little_endian(height_);
    sink.write_4_bytes_little_endian(status_);
	did_.to_data(sink);
}

uint64_t blockchain_did::serialized_size() const
{
    return 4 + tx_point_.serialized_size() + 8 + 4 + did_.serialized_size();
}

#ifdef MVS_DEBUG
std::string blockchain_did::to_string() const
{
    std::ostringstream ss;

    ss << "\t version = " << version_ << "\n"
		<< "\t tx_point = " << tx_point_.to_string() << "\n"
		<< "\t height = " << height_ << "\n"
		<< "\t status = " << get_status_string().c_str() << "\n"
		<< "\t did = " << did_.to_string() << "\n";

    return ss.str();
}

#endif
const uint32_t& blockchain_did::get_version() const
{ 
    return version_;
}
void blockchain_did::set_version(const uint32_t& version_)
{ 
     this->version_ = version_;
}

const output_point& blockchain_did::get_tx_point() const
{ 
    return tx_point_;
}
void blockchain_did::set_tx_point(const output_point& tx_point_)
{ 
     this->tx_point_ = tx_point_;
}

const uint64_t& blockchain_did::get_height() const
{ 
    return height_;
}
void blockchain_did::set_height(const uint64_t& height_)
{ 
     this->height_ = height_;
}

const did_detail& blockchain_did::get_did() const
{ 
    return did_;
}
void blockchain_did::set_did(const did_detail& did_)
{ 
     this->did_ = did_;
}

void blockchain_did::set_status(const uint32_t &status)
{
    this->status_ = status;
}
const uint32_t &blockchain_did::get_status() const
{
    return this->status_;
}

std::string blockchain_did::get_status_string() const
{
    std::string strStatus;
    switch (this->status_)
    {
    case address_invalid:
        strStatus = "invalid";
        break;
    case address_current:
        strStatus = "current";
        break;
    case address_old:
        strStatus = "old";
        break;
    }

    return strStatus;
}
} // namspace chain
} // namspace libbitcoin
