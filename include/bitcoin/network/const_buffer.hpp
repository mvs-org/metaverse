/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
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
#ifndef LIBBITCOIN_NETWORK_CONST_BUFFER_HPP
#define LIBBITCOIN_NETWORK_CONST_BUFFER_HPP

#include <cstddef>
#include <memory>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/network/define.hpp>

namespace libbitcoin {
namespace network {

// A shared boost::asio write buffer, thread safe.
class BCT_API const_buffer
{
public:

    // Required by ConstBufferSequence.
    typedef asio::const_buffer value_type;
    typedef const value_type* const_iterator;

    const_buffer();
    explicit const_buffer(data_chunk&& data);
    explicit const_buffer(const data_chunk& data);

    size_t size() const;
    const_iterator begin() const;
    const_iterator end() const;

private:
    std::shared_ptr<data_chunk> data_;
    value_type buffer_;
};

} // namespace network
} // namespace libbitcoin

#endif
