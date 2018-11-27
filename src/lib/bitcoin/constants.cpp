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
#include <metaverse/bitcoin/constants.hpp>
#include <metaverse/macros_define.hpp>

namespace libbitcoin {

#ifndef PRIVATE_CHAIN

uint32_t coinbase_maturity = 1000;

// PoS
uint64_t pos_enabled_height        = 10;
uint64_t min_pos_lock_value        = 10000 * 100000000ul;
uint64_t min_pos_lock_height       = 10000;
double min_pos_lock_rate           = 0.8;
uint64_t min_pos_value             = 10000 * 100000000ul;
uint64_t min_pos_confirm_height    = 2000;
uint32_t pos_target_timespan       = 24;

#else //PRIVATE_CHAIN

uint32_t coinbase_maturity = 10;

// PoS
uint64_t pos_enabled_height        = 10;
uint64_t min_pos_lock_value        = 10 * 100000000ul;
uint64_t min_pos_lock_height       = 10000;
double min_pos_lock_rate           = 0.8;
uint64_t min_pos_value             = 100 * 100000000ul;
uint64_t min_pos_confirm_height    = 100;
uint32_t pos_target_timespan       = 24;

#endif //PRIVATE_CHAIN

hash_number max_target()
{
    hash_number max_target;
    max_target.set_compact(max_work_bits);
    return max_target;
}

std::string get_developer_community_address(bool is_testnet)
{
    std::string address("MAwLwVGwJyFsTBfNj2j5nCUrQXGVRvHzPh");  // developer-community address for mainnet
    if (is_testnet) {
        address = "tJNo92g6DavpaCZbYjrH45iQ8eAKnLqmms";         // developer-community address for testnet
    }
    return address;
}

} // namespace libbitcoin
