/**
 * Copyright (c) 2011-2021 libbitcoin developers (see AUTHORS)
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
#ifndef MVS_CHAIN_SPEND_HPP
#define MVS_CHAIN_SPEND_HPP

#include <vector>
#include <metaverse/bitcoin/chain/point.hpp>
#include <metaverse/bitcoin/define.hpp>

namespace libbitcoin {
namespace chain {

struct BC_API spend
{
    bool valid;
    uint32_t index;
    hash_digest hash;
};

struct BC_API spend_info
{
    typedef std::vector<spend_info> list;

    input_point point;
    output_point previous_output;
};

} // namespace chain
} // namespace libbitcoin

#endif
