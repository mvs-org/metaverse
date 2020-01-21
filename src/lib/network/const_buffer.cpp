/**
 * Copyright (c) 2019-2020 libbitcoin developers (see AUTHORS)
 *
 * This file is part of metaverse.
 *
 * metaverse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/network/const_buffer.hpp>

#include <memory>
#include <utility>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/define.hpp>

namespace libbitcoin {
namespace network {

const_buffer::const_buffer()
  : data_(std::make_shared<data_chunk>()),
    buffer_(boost::asio::buffer(*data_))
{
}

const_buffer::const_buffer(data_chunk&& data)
  : data_(std::make_shared<data_chunk>(std::forward<data_chunk>(data))),
    buffer_(boost::asio::buffer(*data_))
{
}

const_buffer::const_buffer(const data_chunk& data)
  : data_(std::make_shared<data_chunk>(data)),
    buffer_(boost::asio::buffer(*data_))
{
}

size_t const_buffer::size() const
{
    return data_->size();
}

const_buffer::const_iterator const_buffer::begin() const
{
    return &buffer_;
}

const_buffer::const_iterator const_buffer::end() const
{
    return &buffer_ + 1;
}

} // namespace network
} // namespace libbitcoin
