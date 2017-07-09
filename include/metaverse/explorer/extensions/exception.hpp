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

DEFINE_EXPLORER_EXCEPTION(argument_exceed_limit_exception, 2001);
DEFINE_EXPLORER_EXCEPTION(argument_size_invalid_exception, 2002);
DEFINE_EXPLORER_EXCEPTION(argument_legality_exception, 2003);
DEFINE_EXPLORER_EXCEPTION(argument_dismatch_exception, 2004);

DEFINE_EXPLORER_EXCEPTION(command_dispatch_exception, 2101);
DEFINE_EXPLORER_EXCEPTION(command_issue_asset_exception, 2102);
DEFINE_EXPLORER_EXCEPTION(command_fetch_utxo_exception, 2103);
DEFINE_EXPLORER_EXCEPTION(command_fetch_tx_exception, 2104);
DEFINE_EXPLORER_EXCEPTION(command_get_input_sign_exception, 2105);
DEFINE_EXPLORER_EXCEPTION(command_get_input_set_exception, 2106);
DEFINE_EXPLORER_EXCEPTION(command_get_tx_encode_exception, 2107);
DEFINE_EXPLORER_EXCEPTION(command_get_block_hash_exception, 2108);
DEFINE_EXPLORER_EXCEPTION(command_get_block_header_exception, 2109);

DEFINE_EXPLORER_EXCEPTION(account_existed_exception, 3001);
DEFINE_EXPLORER_EXCEPTION(account_authority_exception, 3002);
DEFINE_EXPLORER_EXCEPTION(account_notfound_exception, 3003);

DEFINE_EXPLORER_EXCEPTION(account_mnemonicword_dismatch_exception, 3101);
DEFINE_EXPLORER_EXCEPTION(account_mnemonicword_empty_exception, 3102);
DEFINE_EXPLORER_EXCEPTION(account_mnemonicword_amount_exception, 3103);
DEFINE_EXPLORER_EXCEPTION(account_mnemonicword_existed_exception, 3104);

DEFINE_EXPLORER_EXCEPTION(account_name_exception, 3201);
DEFINE_EXPLORER_EXCEPTION(account_length_exception, 3202);

DEFINE_EXPLORER_EXCEPTION(address_list_empty_exception, 4001);
DEFINE_EXPLORER_EXCEPTION(address_list_nullptr_exception, 4002);
DEFINE_EXPLORER_EXCEPTION(address_dismatch_account_exception, 4003);
DEFINE_EXPLORER_EXCEPTION(address_amount_exception, 4004);
DEFINE_EXPLORER_EXCEPTION(address_notfound_exception, 4005);

DEFINE_EXPLORER_EXCEPTION(address_invalid_exception, 4010);
DEFINE_EXPLORER_EXCEPTION(toaddress_empty_exception, 4011);
DEFINE_EXPLORER_EXCEPTION(toaddress_invalid_exception, 4012);
DEFINE_EXPLORER_EXCEPTION(toaddress_unrecognized_exception, 4013);
DEFINE_EXPLORER_EXCEPTION(fromaddress_empty_exception, 4014);
DEFINE_EXPLORER_EXCEPTION(fromaddress_invalid_exception, 4015);
DEFINE_EXPLORER_EXCEPTION(fromaddress_unrecognized_exception, 4016);

DEFINE_EXPLORER_EXCEPTION(asset_lack_exception, 5001);
DEFINE_EXPLORER_EXCEPTION(asset_amount_exception, 5002);
DEFINE_EXPLORER_EXCEPTION(asset_notfound_exception, 5003);
DEFINE_EXPLORER_EXCEPTION(asset_type_exception, 5004);
DEFINE_EXPLORER_EXCEPTION(asset_exchange_poundage_exception, 5005);
DEFINE_EXPLORER_EXCEPTION(asset_issue_poundage_exception, 5006);
DEFINE_EXPLORER_EXCEPTION(asset_description_length_exception, 5007);
DEFINE_EXPLORER_EXCEPTION(asset_symbol_duplicate_exception, 5008);
DEFINE_EXPLORER_EXCEPTION(asset_symbol_existed_exception, 5009);
DEFINE_EXPLORER_EXCEPTION(asset_symbol_notfound_exception, 5010);
DEFINE_EXPLORER_EXCEPTION(asset_symbol_length_exception, 5011);
DEFINE_EXPLORER_EXCEPTION(asset_symbol_name_exception, 5012);

DEFINE_EXPLORER_EXCEPTION(balance_lack_exception, 5101);
DEFINE_EXPLORER_EXCEPTION(account_etp_lack_exception, 5111);
DEFINE_EXPLORER_EXCEPTION(utxo_etp_lack_exception, 5121);

DEFINE_EXPLORER_EXCEPTION(tx_io_exception, 5301);
DEFINE_EXPLORER_EXCEPTION(tx_source_exception, 5302);
DEFINE_EXPLORER_EXCEPTION(tx_sign_exception, 5303);
DEFINE_EXPLORER_EXCEPTION(tx_validate_exception, 5304);
DEFINE_EXPLORER_EXCEPTION(tx_broadcast_exception, 5305);
DEFINE_EXPLORER_EXCEPTION(tx_notfound_exception, 5306);
DEFINE_EXPLORER_EXCEPTION(tx_attachment_value_exception, 5307);
DEFINE_EXPLORER_EXCEPTION(tx_fetch_exception, 5308);


DEFINE_EXPLORER_EXCEPTION(redeem_script_empty_exception, 5401);
DEFINE_EXPLORER_EXCEPTION(redeem_script_data_exception, 5402);
DEFINE_EXPLORER_EXCEPTION(redeem_script_pattern_exception, 5403);

DEFINE_EXPLORER_EXCEPTION(query_block_exception, 6101);
DEFINE_EXPLORER_EXCEPTION(query_last_block_exception, 6102);
DEFINE_EXPLORER_EXCEPTION(get_account_address_exception, 6103);
DEFINE_EXPLORER_EXCEPTION(set_deposit_period_exception, 6104);

DEFINE_EXPLORER_EXCEPTION(block_height_exception, 7001);

DEFINE_EXPLORER_EXCEPTION(connection_exception, 8001);
DEFINE_EXPLORER_EXCEPTION(format_timestamp_exception, 8002);
DEFINE_EXPLORER_EXCEPTION(locktime_over_exception, 8003);
DEFINE_EXPLORER_EXCEPTION(locktime_invalid_exception, 8004);


DEFINE_EXPLORER_EXCEPTION(seed_size_exception, 9001);
DEFINE_EXPLORER_EXCEPTION(seed_length_exception, 9002);
DEFINE_EXPLORER_EXCEPTION(mnemonicwords_amount_exception, 9003);
DEFINE_EXPLORER_EXCEPTION(mnemonicwords_content_exception, 9004);
DEFINE_EXPLORER_EXCEPTION(hd_length_exception, 9005);
DEFINE_EXPLORER_EXCEPTION(hd_key_exception, 9006);


} //namespace explorer
} //namespace libbitcoin
#endif /* INCLUDE_METAVERSE_EXPLORER_UTILITIES_EXCEPTION_HPP_ */
