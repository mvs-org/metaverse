/**
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS)
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

#ifndef INCLUDE_BITCOIN_BITCOIN_UTILITY_PATH_HPP_
#define INCLUDE_BITCOIN_BITCOIN_UTILITY_PATH_HPP_

#include <boost/filesystem.hpp>

namespace libbitcoin
{
	boost::filesystem::path default_data_path();

	boost::filesystem::path webpage_path();

}//namespace libbitcoin


#endif /* INCLUDE_BITCOIN_BITCOIN_UTILITY_PATH_HPP_ */
