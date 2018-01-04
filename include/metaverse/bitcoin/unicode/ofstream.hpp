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
#ifndef MVS_OFSTREAM_HPP
#define MVS_OFSTREAM_HPP

#include <fstream>
#include <string>
#include <metaverse/bitcoin/define.hpp>

namespace libbitcoin {

const uint64_t LOG_MAX_SIZE = 1024 * 1024 * 512;
/**
 * Use bc::ofstream in place of std::ofstream.
 * This provides utf8 to utf16 path translation for Windows.
 */
class BC_API ofstream
  : public std::ofstream
{
public:
    /**
     * Construct bc::ofstream.
     * @param[in]  path  The utf8 path to the file.
     * @param[in]  mode  The file opening mode.
     */
    ofstream(const std::string& path,
        std::ofstream::openmode mode = std::ofstream::out);

    std::string path() const;
    uint64_t max_size() const;
    uint64_t& current_size();
    uint64_t increment(uint64_t size);

private:
    std::string path_;
    uint64_t current_size_;
    uint64_t max_size_;
};

inline std::string ofstream::path() const
{
    return path_;
}

inline uint64_t ofstream::max_size() const
{
    return max_size_;
}

inline uint64_t& ofstream::current_size()
{
    return current_size_;
}

} // namespace libbitcoin

#endif
