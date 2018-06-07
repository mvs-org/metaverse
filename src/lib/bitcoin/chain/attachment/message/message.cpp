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
#include <metaverse/bitcoin/chain/attachment/message/message.hpp>
#include <sstream>
#include <boost/iostreams/stream.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>

namespace libbitcoin {
namespace chain {

blockchain_message::blockchain_message(): content_("")
{
}
blockchain_message::blockchain_message(std::string content):
    content_(content)
{

}

blockchain_message blockchain_message::factory_from_data(const data_chunk& data)
{
    blockchain_message instance;
    instance.from_data(data);
    return instance;
}

blockchain_message blockchain_message::factory_from_data(std::istream& stream)
{
    blockchain_message instance;
    instance.from_data(stream);
    return instance;
}

blockchain_message blockchain_message::factory_from_data(reader& source)
{
    blockchain_message instance;
    instance.from_data(source);
    return instance;
}

void blockchain_message::reset()
{
    content_ = "";
}

static size_t get_string_serialized_size(const std::string& str)
{
    size_t length = str.size();
    if (length < 0xfd) {
        return length + 1;
    }
    if (length <= 0xffff) {
        return length + 3;
    }
    if (length <= 0xffffffff) {
        return length + 5;
    }
    return length + 9;
}

bool blockchain_message::is_valid() const
{
    return !(content_.empty()
            // add 1 here to prevent content_'s size == 253
            || get_string_serialized_size(content_) + 1 > BLOCKCHAIN_MESSAGE_FIX_SIZE);
}

bool blockchain_message::from_data(const data_chunk& data)
{
    data_source istream(data);
    return from_data(istream);
}

bool blockchain_message::from_data(std::istream& stream)
{
    istream_reader source(stream);
    return from_data(source);
}

bool blockchain_message::from_data(reader& source)
{
    reset();
    content_ = source.read_string();
    auto result = static_cast<bool>(source);

    return result;
}

data_chunk blockchain_message::to_data() const
{
    data_chunk data;
    data_sink ostream(data);
    to_data(ostream);
    ostream.flush();
    BITCOIN_ASSERT(data.size() == serialized_size());
    return data;
}

void blockchain_message::to_data(std::ostream& stream) const
{
    ostream_writer sink(stream);
    to_data(sink);
}

void blockchain_message::to_data(writer& sink) const
{
    sink.write_string(content_);
}

uint64_t blockchain_message::serialized_size() const
{
    size_t len = get_string_serialized_size(content_);
    return std::min(len, BLOCKCHAIN_MESSAGE_FIX_SIZE);
}

std::string blockchain_message::to_string() const
{
    std::ostringstream ss;
    ss << "\t content = " << content_ << "\n";

    return ss.str();
}
const std::string& blockchain_message::get_content() const
{
    return content_;
}

void blockchain_message::set_content(const std::string& content)
{
    size_t len = get_string_serialized_size(content_);
    len = std::min(len, BLOCKCHAIN_MESSAGE_FIX_SIZE);
    this->content_ = content.substr(0, len);
}

} // namspace chain
} // namspace libbitcoin
