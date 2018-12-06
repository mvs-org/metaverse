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
const uint64_t future_blocktime_fork_height = 1030000;

// PoS
const uint64_t pos_enabled_height        = max_uint64;
const uint32_t pos_coinstake_max_utxos   = 10;
const uint64_t pos_lock_min_value        = 10000 * 100000000ul;
const uint64_t pos_lock_min_height       = 100000;
const uint64_t pos_lock_gap_height       = 10000;
const uint64_t pos_stake_min_value       = 10000 * 100000000ul;
const uint64_t pos_stake_min_height      = 1000;
const double pos_stake_factor            = 1;
const uint32_t block_timespan_window     = 28;

#else //PRIVATE_CHAIN

uint32_t coinbase_maturity = 10;
const uint64_t future_blocktime_fork_height = 10;

// PoS
const uint64_t pos_enabled_height        = 350;
const uint32_t pos_coinstake_max_utxos   = 10;
const uint64_t pos_lock_min_value        = 10 * 100000000ul;
const uint64_t pos_lock_min_height       = 100000;
const uint64_t pos_lock_gap_height       = 10000;
const uint64_t pos_stake_min_value       = 100 * 100000000ul;
const uint64_t pos_stake_min_height      = 100;
const double pos_stake_factor            = 10;
const uint32_t block_timespan_window     = 28;

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
