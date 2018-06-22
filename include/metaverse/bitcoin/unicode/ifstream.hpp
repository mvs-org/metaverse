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
#ifndef MVS_IFSTREAM_HPP
#define MVS_IFSTREAM_HPP

#include <fstream>
#include <string>
#include <metaverse/bitcoin/define.hpp>

namespace libbitcoin {

/**
 * Use bc::ifstream in place of std::ifstream.
 * This provides utf8 to utf16 path translation for Windows.
 */
class BC_API ifstream
  : public std::ifstream
{
public:
    /**
     * Construct bc::ifstream.
     * @param[in]  path  The utf8 path to the file.
     * @param[in]  mode  The file opening mode.
     */
    ifstream(const std::string& path,
        std::ifstream::openmode mode=std::ifstream::in);
};

} // namespace libbitcoin

#endif
