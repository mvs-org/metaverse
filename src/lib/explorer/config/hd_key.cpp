/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse-explorer.
 *
 * metaverse-explorer is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
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
#include <metaverse/explorer/config/hd_key.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <boost/program_options.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/explorer/define.hpp>

using namespace po;

namespace libbitcoin {
namespace explorer {
namespace config {

hd_key::hd_key(const std::string& base58)
{
    std::stringstream(base58) >> *this;
}

uint32_t hd_key::version() const
{
    return from_big_endian_unsafe<uint32_t>(value_.begin());
}

hd_key::operator const wallet::hd_key&() const
{
    return value_;
}

std::istream& operator>>(std::istream& input, hd_key& argument)
{
    std::string base58;
    input >> base58;

    data_chunk value;
    if (!decode_base58(value, base58) || value.size() != wallet::hd_key_size)
    {
        BOOST_THROW_EXCEPTION(invalid_option_value(base58));
    }

    std::copy(value.begin(), value.end(), argument.value_.begin());
    return input;
}

} // namespace config
} // namespace explorer
} // namespace libbitcoin
