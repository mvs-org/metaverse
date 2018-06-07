/**
 * Copyright (c) 2011-2013 libbitcoin developers (see AUTHORS)
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
#ifndef MVS_VARIABLE_UINT_SIZE_HPP
#define MVS_VARIABLE_UINT_SIZE_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <metaverse/bitcoin/define.hpp>

namespace libbitcoin {

BC_API size_t variable_uint_size(uint64_t value);
BC_API size_t get_string_serialized_size(const std::string& str);

} // namespace libbitcoin

#endif

