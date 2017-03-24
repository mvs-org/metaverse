/**
 * Copyright (c) 2011-2015 metaverse developers (see AUTHORS)
 *
 * This file is part of mvs-node.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
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
#ifndef MVS_CHAIN_ATTACHMENT_INTERFACE_HPP
#define MVS_CHAIN_ATTACHMENT_INTERFACE_HPP

#include <cstdint>
#include <istream>
#include <metaverse/lib/bitcoin/define.hpp>
#include <metaverse/lib/bitcoin/utility/reader.hpp>
#include <metaverse/lib/bitcoin/utility/writer.hpp>

namespace libbitcoin {
namespace chain {

class BC_API ser_deser_interface
{
public:

    virtual bool from_data(reader& source) = 0;
    virtual void to_data(writer& sink) = 0 ;
    virtual std::string to_string() = 0 ;
    virtual bool is_valid() const  = 0;
    virtual void reset()  = 0;
    virtual uint64_t serialized_size() = 0 ;
};

} // namespace chain
} // namespace libbitcoin

#endif

