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

console_result capture_excode(std::stringstream&, std::pair<uint32_t, std::string>&);

DEFINE_EXPLORER_EXCEPTION(fatal_exception, 1001);

DEFINE_EXPLORER_EXCEPTION(argument_exceed_limit_exception, 2001);
DEFINE_EXPLORER_EXCEPTION(argument_size_invalid_exception, 2002);
DEFINE_EXPLORER_EXCEPTION(argument_legality_exception, 2003);
DEFINE_EXPLORER_EXCEPTION(argument_dismatch_exception, 2004);

DEFINE_EXPLORER_EXCEPTION(account_existed_exception, 3001);
DEFINE_EXPLORER_EXCEPTION(account_authority_exception, 3002);
DEFINE_EXPLORER_EXCEPTION(account_notfound_exception, 3003);

DEFINE_EXPLORER_EXCEPTION(account_mnemonicword_dismatch_exception, 3101);
DEFINE_EXPLORER_EXCEPTION(account_mnemonicword_empty_exception, 3102);
DEFINE_EXPLORER_EXCEPTION(account_mnemonicword_amount_exception, 3103);
DEFINE_EXPLORER_EXCEPTION(account_mnemonicword_existed_exception, 3104);

DEFINE_EXPLORER_EXCEPTION(account_name_exception, 3201);
DEFINE_EXPLORER_EXCEPTION(account_length_exception, 3202);
DEFINE_EXPLORER_EXCEPTION(account_address_get_exception, 3203);


DEFINE_EXPLORER_EXCEPTION(address_list_empty_exception, 4001);
DEFINE_EXPLORER_EXCEPTION(address_list_nullptr_exception, 4002);
DEFINE_EXPLORER_EXCEPTION(address_dismatch_account_exception, 4003);
DEFINE_EXPLORER_EXCEPTION(address_amount_exception, 4004);
DEFINE_EXPLORER_EXCEPTION(address_notfound_exception, 4005);
DEFINE_EXPLORER_EXCEPTION(address_generate_exception, 4005);


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

DEFINE_EXPLORER_EXCEPTION(block_height_get_exception, 5101);
DEFINE_EXPLORER_EXCEPTION(block_last_height_get_exception, 5102);
DEFINE_EXPLORER_EXCEPTION(block_height_exception, 5103);
DEFINE_EXPLORER_EXCEPTION(block_hash_get_exception, 5104);
DEFINE_EXPLORER_EXCEPTION(block_header_get_exception, 5105);

DEFINE_EXPLORER_EXCEPTION(multisig_cosigne_exception, 5201);
DEFINE_EXPLORER_EXCEPTION(multisig_exist_exception, 5202);
DEFINE_EXPLORER_EXCEPTION(multisig_notfound_exception, 5203);
DEFINE_EXPLORER_EXCEPTION(multisig_script_exception, 5204);
DEFINE_EXPLORER_EXCEPTION(multisig_index_exception, 5205);
DEFINE_EXPLORER_EXCEPTION(signature_amount_exception, 5220);
DEFINE_EXPLORER_EXCEPTION(pubkey_amount_exception, 5230);
DEFINE_EXPLORER_EXCEPTION(pubkey_dismatch_exception, 5231);
DEFINE_EXPLORER_EXCEPTION(prikey_notfound_exception, 5232);


DEFINE_EXPLORER_EXCEPTION(tx_io_exception, 5301);
DEFINE_EXPLORER_EXCEPTION(tx_source_exception, 5302);
DEFINE_EXPLORER_EXCEPTION(tx_sign_exception, 5303);
DEFINE_EXPLORER_EXCEPTION(tx_validate_exception, 5304);
DEFINE_EXPLORER_EXCEPTION(tx_broadcast_exception, 5305);
DEFINE_EXPLORER_EXCEPTION(tx_notfound_exception, 5306);
DEFINE_EXPLORER_EXCEPTION(tx_attachment_value_exception, 5307);
DEFINE_EXPLORER_EXCEPTION(tx_fetch_exception, 5308);
DEFINE_EXPLORER_EXCEPTION(tx_send_exception, 5309);
DEFINE_EXPLORER_EXCEPTION(tx_encode_get_exception, 5310);
DEFINE_EXPLORER_EXCEPTION(tx_decode_get_exception, 5311);

DEFINE_EXPLORER_EXCEPTION(redeem_script_empty_exception, 5401);
DEFINE_EXPLORER_EXCEPTION(redeem_script_data_exception, 5402);
DEFINE_EXPLORER_EXCEPTION(redeem_script_pattern_exception, 5403);


DEFINE_EXPLORER_EXCEPTION(set_deposit_period_exception, 6001);
DEFINE_EXPLORER_EXCEPTION(fetch_utxo_exception, 6002);
DEFINE_EXPLORER_EXCEPTION(get_input_sign_exception, 6003);
DEFINE_EXPLORER_EXCEPTION(get_input_set_exception, 6004);
DEFINE_EXPLORER_EXCEPTION(lack_balance_exception, 6005);
DEFINE_EXPLORER_EXCEPTION(lack_account_etp_exception, 6006);
DEFINE_EXPLORER_EXCEPTION(lack_utxo_etp_exception, 6007);
DEFINE_EXPLORER_EXCEPTION(encode_exception, 6008);
DEFINE_EXPLORER_EXCEPTION(ec_to_address_exception, 6009);
DEFINE_EXPLORER_EXCEPTION(ec_to_public_exception, 6010);
DEFINE_EXPLORER_EXCEPTION(mnemonic_new_exception, 6011);
DEFINE_EXPLORER_EXCEPTION(mnemonic_to_seed_exception, 6012);


DEFINE_EXPLORER_EXCEPTION(connection_exception, 8001);
DEFINE_EXPLORER_EXCEPTION(format_timestamp_exception, 8002);
DEFINE_EXPLORER_EXCEPTION(locktime_over_exception, 8003);
DEFINE_EXPLORER_EXCEPTION(locktime_invalid_exception, 8004);

DEFINE_EXPLORER_EXCEPTION(seed_exception, 9001);
DEFINE_EXPLORER_EXCEPTION(seed_size_exception, 9001);
DEFINE_EXPLORER_EXCEPTION(seed_length_exception, 9002);
DEFINE_EXPLORER_EXCEPTION(mnemonicwords_amount_exception, 9003);
DEFINE_EXPLORER_EXCEPTION(mnemonicwords_content_exception, 9004);
DEFINE_EXPLORER_EXCEPTION(hd_length_exception, 9101);
DEFINE_EXPLORER_EXCEPTION(hd_key_exception, 9102);
DEFINE_EXPLORER_EXCEPTION(hd_new_exception, 9103);
DEFINE_EXPLORER_EXCEPTION(hd_private_new_exception, 9104);
DEFINE_EXPLORER_EXCEPTION(hd_to_ec_exception, 9105);

} //namespace explorer
} //namespace libbitcoin
#endif /* INCLUDE_METAVERSE_EXPLORER_UTILITIES_EXCEPTION_HPP_ */
