/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/bitcoin/utility/variable_uint_size.hpp>

namespace libbitcoin {

size_t variable_uint_size(uint64_t value)
{
    if (value < 0xfd)
        return 1;
    else if (value <= 0xffff)
        return 3;
    else if (value <= 0xffffffff)
        return 5;
    else
        return 9;
}

size_t variable_string_size(const std::string& str)
{
    size_t length = str.size();
    length += variable_uint_size(length);
    return length;
}

std::string limit_size_string(const std::string& str, size_t limit_size)
{
    if (str.size() > limit_size) {
        return str.substr(0, limit_size);
    }

    return str;
}

} // namespace libbitcoin
