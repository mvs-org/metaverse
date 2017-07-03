/*
 * exception.hpp
 *
 *  Created on: Jun 5, 2017
 *      Author: jiang
 */

#ifndef INCLUDE_METAVERSE_EXPLORER_UTILITIES_EXCEPTION_HPP_
#define INCLUDE_METAVERSE_EXPLORER_UTILITIES_EXCEPTION_HPP_

#include <boost/format.hpp>

#define DEFINE_EXPLORER_EXCEPTION(class_name, exception_code)     \
class class_name : public explorer_exception                      \
{                                                                 \
public:                                                           \
	class_name(const std::string& message) :                      \
		explorer_exception(exception_code, message)               \
	{                                                             \
	}                                                             \
}

#define DEFINE_EXPLORER_EXCEPTION2(class_name)                    \
class class_name : public explorer_exception                      \
{                                                                 \
public:                                                           \
	class_name(uint32_t code, const std::string& message) :       \
		explorer_exception(code, message)                         \
	{                                                             \
	}                                                             \
}

namespace libbitcoin {
namespace explorer {

class explorer_exception : public std::exception
{
public:
	explorer_exception(uint32_t code, const std::string& message);

	virtual ~explorer_exception() = default;
	uint32_t code() const { return code_; }
	const std::string& message() const { return message_; }
	virtual const char* what() const noexcept override { return message_.data(); }
private:
	uint32_t code_;
	std::string message_;
};

std::ostream& operator<<(std::ostream& out, const explorer_exception& ex);

DEFINE_EXPLORER_EXCEPTION(invalid_account_exception, 1);
DEFINE_EXPLORER_EXCEPTION(account_not_exist_exception, 2);
DEFINE_EXPLORER_EXCEPTION(address_mismatch_account_exception, 3);
DEFINE_EXPLORER_EXCEPTION(invalid_argument_exception, 4);
DEFINE_EXPLORER_EXCEPTION(tx_not_found_exception, 5);
DEFINE_EXPLORER_EXCEPTION(admin_required_exception, 7);
DEFINE_EXPLORER_EXCEPTION(last_word_mismatch_exception, 8);
DEFINE_EXPLORER_EXCEPTION(invalid_address_exception, 9);
DEFINE_EXPLORER_EXCEPTION(query_block_height_failed_exception, 10);
DEFINE_EXPLORER_EXCEPTION(invalid_block_height_exception, 11);
DEFINE_EXPLORER_EXCEPTION(no_asset_found_exception, 12);


DEFINE_EXPLORER_EXCEPTION(account_existed_exception, 2001);


DEFINE_EXPLORER_EXCEPTION(argument_exceed_limit_exception, 3001);
DEFINE_EXPLORER_EXCEPTION(argument_size_invalid_exception, 3002);
DEFINE_EXPLORER_EXCEPTION(argument_address_invalid_exception, 3002);

DEFINE_EXPLORER_EXCEPTION(query_block_exception, 3102);
DEFINE_EXPLORER_EXCEPTION(query_last_block_exception, 3102);
DEFINE_EXPLORER_EXCEPTION(get_account_address_exception, 3102); 
DEFINE_EXPLORER_EXCEPTION(set_deposit_period_exception, 3102);





DEFINE_EXPLORER_EXCEPTION(address_list_empty_exception, 4001);
DEFINE_EXPLORER_EXCEPTION(address_to_exception, 4002);
DEFINE_EXPLORER_EXCEPTION(address_from_exception, 4003);
DEFINE_EXPLORER_EXCEPTION(address_deposit_exception, 4003);


DEFINE_EXPLORER_EXCEPTION(connection_exception, 5001);
DEFINE_EXPLORER_EXCEPTION(over_locktime_exception, 5002);
DEFINE_EXPLORER_EXCEPTION(command_dispatch_exception, 5003);
DEFINE_EXPLORER_EXCEPTION(command_get_input_sign_exception, 5101);
DEFINE_EXPLORER_EXCEPTION(command_get_input_set_exception, 5102);
DEFINE_EXPLORER_EXCEPTION(command_fetch_utxo_exception, 5102);
DEFINE_EXPLORER_EXCEPTION(command_fetch_tx_exception, 5102);
DEFINE_EXPLORER_EXCEPTION(command_get_tx_encode_exception, 5102);
DEFINE_EXPLORER_EXCEPTION(command_get_block_hash_exception, 5102);
DEFINE_EXPLORER_EXCEPTION(command_get_block_header_exception, 5102);






DEFINE_EXPLORER_EXCEPTION(fee_range_exception, 6001);
DEFINE_EXPLORER_EXCEPTION(balance_lack_exception, 6101);
DEFINE_EXPLORER_EXCEPTION(account_etp_lack_exception, 6201);
DEFINE_EXPLORER_EXCEPTION(utxo_etp_lack_exception, 6202);

DEFINE_EXPLORER_EXCEPTION(asset_lack_exception, 6301);
DEFINE_EXPLORER_EXCEPTION(asset_issue_exception, 6302);
DEFINE_EXPLORER_EXCEPTION(asset_issue_count_exception, 6303);


DEFINE_EXPLORER_EXCEPTION(attachment_value_invalid_exception, 6401);

DEFINE_EXPLORER_EXCEPTION(tx_io_exception, 7001);
DEFINE_EXPLORER_EXCEPTION(tx_source_exception, 7002);
DEFINE_EXPLORER_EXCEPTION(tx_sign_exception, 7003);
DEFINE_EXPLORER_EXCEPTION(tx_validate_exception, 7004);
DEFINE_EXPLORER_EXCEPTION(tx_broadcast_exception, 7005);

DEFINE_EXPLORER_EXCEPTION(redeem_script_empty_exception, 7101);
DEFINE_EXPLORER_EXCEPTION(redeem_script_data_exception, 7102);
DEFINE_EXPLORER_EXCEPTION(redeem_script_pattern_exception, 7103);













DEFINE_EXPLORER_EXCEPTION2(address_history_fetch_exception);
} //namespace explorer
} //namespace libbitcoin
#endif /* INCLUDE_METAVERSE_EXPLORER_UTILITIES_EXCEPTION_HPP_ */
