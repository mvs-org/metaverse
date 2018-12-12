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
#ifndef MVS_ERROR_HPP
#define MVS_ERROR_HPP

#include <string>
#include <system_error>
#include <boost/system/error_code.hpp>
#include <metaverse/bitcoin/define.hpp>

namespace libbitcoin {

/// Console result codes, positive values are domain-specific.
enum console_result : int
{
    failure = -1,
    okay = 0,
    invalid = 1,
    cmd_output = 2
};

/// Alias for error code declarations.
typedef std::error_code code;

/// Alias for boost error code declarations.
typedef boost::system::error_code boost_code;

namespace error {

// The numeric values of these codes may change without notice.
enum error_code_t
{
    success = 0,

    // network errors
    service_stopped,                // 1
    operation_failed,

    // blockchain errors
    not_found,
    duplicate,
    unspent_output,                 // 5
    unsupported_script_pattern,

    // network errors (more)
    resolve_failed,
    network_unreachable,
    address_in_use,
    listen_failed,                  // 10
    accept_failed,
    bad_stream,
    channel_timeout,

    // transaction pool
    blockchain_reorganized,
    pool_filled,                    // 15

    // validate tx
    coinbase_transaction,
    is_not_standard,
    double_spend,
    input_not_found,
    invalid_input_script_lock_height, // 20
    invalid_output_script_lock_height,

    // check_transaction()
    empty_transaction,
    output_value_overflow,
    invalid_coinbase_script_size,
    previous_output_null,           // 25
    script_not_standard,
    transaction_version_error,

    // validate block
    previous_block_invalid,

    // check_block()
    size_limits,
    proof_of_work,                  // 30
    futuristic_timestamp,
    first_not_coinbase,
    extra_coinbases,
    too_many_sigs,
    merkle_mismatch,                // 35

    // accept_block()
    incorrect_proof_of_work,
    timestamp_too_early,
    non_final_transaction,
    checkpoints_failed,
    old_version_block,              // 40
    coinbase_height_mismatch,

    // connect_block()
    duplicate_or_spent,
    validate_inputs_failed,
    fees_out_of_range,
    coinbase_too_large,             // 45
    invalid_coinage_reward_coinbase,

    // file system errors
    file_system,

    // unknown errors
    unknown,

    // network errors (more)
    address_blocked,
    channel_stopped,                // 50
    not_satisfied,
    mock,

    // asset check
    asset_amount_overflow,
    asset_amount_not_equal,
    asset_symbol_not_match,         // 55
    asset_symbol_invalid,
    asset_address_not_match,
    asset_exist,
    asset_not_exist,
    asset_issue_error,              // 60
    asset_secondaryissue_error,
    asset_secondaryissue_share_not_enough,
    asset_secondaryissue_threshold_invalid,

    //syn block
    fetch_more_block,
    bad_magic,                      // 65

    // did check
    did_symbol_not_match,
    did_symbol_invalid,
    did_exist,
    address_registered_did,
    did_func_not_actived,           // 70
    did_address_not_match,
    did_address_needed,
    did_not_exist,
    did_multi_type_exist,
    did_input_error,                // 75
    attenuation_model_param_error,

    // cert check
    asset_cert_error,
    asset_cert_exist,
    asset_cert_not_exist,
    asset_cert_not_owned,           // 80
    asset_cert_not_provided,
    asset_cert_issue_error,
    asset_did_registerr_not_match,

    attachment_invalid,
    nova_feature_not_activated,     // 85

    // identifier asset
    mit_error,
    mit_exist,
    mit_register_error,
    mit_symbol_invalid,
    mit_not_exist,                  // 90

    sequence_locked,
    sync_disabled,
    block_version_not_match,
    witness_sign_invalid,
    witness_mismatch,               // 95
    witness_vote_error,
    witness_update_error,

    proof_of_stake,
    miss_coinstake,
    illegal_coinstake,
    extra_coinstakes,
    coinstake_version_invalid,
    cointstake_signature_invalid,
    check_pos_genesis_error,
    delegate_mismatch_error

};

enum error_condition_t
{
    //// validate
    //validate_failed = 1,
    //forced_removal
};

BC_API code make_error_code(error_code_t e);
BC_API std::error_condition make_error_condition(error_condition_t e);
BC_API error_code_t boost_to_error_code(const boost_code& ec);

} // namespace error
} // namespace libbitcoin

namespace std {

template <>
struct is_error_code_enum<bc::error::error_code_t>
  : public true_type
{
};

template <>
struct is_error_condition_enum<bc::error::error_condition_t>
  : public true_type
{
};

} // namespace std

#endif
