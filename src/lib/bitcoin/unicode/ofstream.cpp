/**
 * Copyright (c) 2019-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2019-2020 metaverse core developers (see MVS-AUTHORS)
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
#include <metaverse/bitcoin/unicode/ofstream.hpp>

#include <fstream>
#include <string>
#include <metaverse/bitcoin/unicode/unicode.hpp>
#include <boost/filesystem.hpp>

namespace libbitcoin {

// Construct bc::ofstream.
ofstream::ofstream(const std::string& path, std::ofstream::openmode mode)
#ifdef _MSC_VER
    : std::ofstream(bc::to_utf16(path), mode), max_size_(LOG_MAX_SIZE), path_(path)
#else
    : std::ofstream(path, mode), max_size_(LOG_MAX_SIZE), path_(path)
#endif
{
    current_size_ = boost::filesystem::file_size(path);
}

uint64_t ofstream::increment(uint64_t size)
{
    current_size_ += size;
    return current_size_;
}


} // namespace libbitcoin
