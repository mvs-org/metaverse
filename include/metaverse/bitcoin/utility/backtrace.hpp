/**
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INCLUDE_BITCOIN_BITCOIN_UTILITY_BACKTRACE_HPP_
#define INCLUDE_BITCOIN_BITCOIN_UTILITY_BACKTRACE_HPP_

#include <iostream>
#include <string>

namespace libbitcoin
{
void back_trace(std::ostream& os);
void do_backtrace(const std::string& name);
}//namespace libbitcoin

#endif /* INCLUDE_BITCOIN_BITCOIN_UTILITY_BACKTRACE_HPP_ */
