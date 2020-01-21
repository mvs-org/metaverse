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


void blockchain_message::reset()
{
    content_ = "";
}

bool blockchain_message::is_valid() const
{
    return !(content_.empty()
            // add 1 here to prevent content_'s size == 253
            || variable_string_size(content_) + 1 > BLOCKCHAIN_MESSAGE_FIX_SIZE);
}

bool blockchain_message::from_data_t(reader& source)
{
    reset();
    content_ = source.read_string();
    auto result = static_cast<bool>(source);

    return result;
}

void blockchain_message::to_data_t(writer& sink) const
{
    sink.write_string(content_);
}

uint64_t blockchain_message::serialized_size() const
{
    size_t len = variable_string_size(content_);
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
    content_ = limit_size_string(content, BLOCKCHAIN_MESSAGE_FIX_SIZE);
}

} // namspace chain
} // namspace libbitcoin
