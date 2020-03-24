/**
 * Copyright (c) 2011-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2020 metaverse core developers (see MVS-AUTHORS)
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

//==============================================================================
//
//==============================================================================

#ifdef PRIVATE_CHAIN

uint32_t coinbase_maturity                      = 10;
uint64_t pos_enabled_height                     = 5000;
uint64_t domain_enabled_height                  = 0;

const uint64_t future_blocktime_fork_height     = 0;
const uint64_t nova_enabled_height              = 0;

#else //PRIVATE_CHAIN

uint32_t coinbase_maturity                      = 1000;
uint64_t pos_enabled_height                     = 1924000; // hard-fork-of-MPC1
uint64_t domain_enabled_height                  = 2800000; // hard-fork-of-MPC1.5

const uint64_t future_blocktime_fork_height     = 1030000;
const uint64_t nova_enabled_height              = 1270000;

#endif //PRIVATE_CHAIN


//==============================================================================
// constants
//==============================================================================

const size_t relative_locktime_min_version      = 2;

// POS
const bool enable_max_successive_height         = true;
const uint32_t pow_max_successive_height        = 60;
const uint32_t pos_max_successive_height        = 24;

const uint64_t pos_genesis_reward               = coin_price(0X1076F8E);
const uint32_t pos_coinstake_max_utxos          = 10;
const uint64_t pos_lock_min_value               = coin_price(1000);
const uint64_t pos_lock_min_height              = 24000;
const uint64_t pos_lock_gap_height              = 1000;
const uint64_t pos_stake_min_value              = coin_price(1000);
const double   pos_stake_factor                 = 30;
const uint32_t block_timespan_window            = 38;

const std::string witness_cert_prefix("MVS.WITNESS.");
const uint32_t witness_cert_mars_value          = 30;
const uint32_t witness_cert_count               = 23;
const uint32_t secondary_witness_cert_min       = 23;
const uint32_t secondary_witness_cert_max       = 46;
const uint32_t secondary_witness_cert_expiration    = 2000000;


//==============================================================================
// functions
//==============================================================================

bool is_relative_locktime_time_locked(uint32_t raw_value)
{
    return (raw_value & relative_locktime_time_locked) != 0;
}

uint32_t get_relative_locktime_locked_heights(uint32_t raw_value)
{
    if (is_relative_locktime_time_locked(raw_value)) {
        return 0;
    }
    return raw_value & relative_locktime_mask;
}

uint32_t get_relative_locktime_locked_seconds(uint32_t raw_value)
{
    if (!is_relative_locktime_time_locked(raw_value)) {
        return 0;
    }
    return (raw_value & relative_locktime_mask) << relative_locktime_seconds_shift;
}

std::string get_genesis_address(bool is_testnet)
{
    if (is_testnet) {
        return "tPd41bKLJGf1C5RRvaiV2mytqZB6WfM1vR";            // for testnet
    }

#ifdef PRIVATE_CHAIN
    return "MCCu9WmXuTooSEoMHX1aN82b7syaJD65XM";                // for private net
#else
    return "MGqHvbaH9wzdr6oUDFz4S1HptjoKQcjRve";                // for mainnet
#endif
}

std::string get_foundation_address(bool is_testnet)
{
    if (is_testnet) {
        return "tBELxsiiaMVGQcY2Apf7hmzAaipD4YWTTj";            // for testnet
    }

#ifdef PRIVATE_CHAIN
    return "MCCu9WmXuTooSEoMHX1aN82b7syaJD65XM";                // for private net
#else
    return "MSCHL3unfVqzsZbRVCJ3yVp7RgAmXiuGN3";                // for mainnet
#endif
}

std::string get_developer_community_address(bool is_testnet)
{
    if (is_testnet) {
        return "tBELxsiiaMVGQcY2Apf7hmzAaipD4YWTTj";            // for testnet
    }

    return "MAwLwVGwJyFsTBfNj2j5nCUrQXGVRvHzPh";                // for mainnet
}

} // namespace libbitcoin
