# return code

success = 0

unknown_error_exception = 500
fatal_exception = 1001
connection_exception = 1011
session_expired_exception = 1012

invalid_command_exception = 1020
command_params_exception = 1021
command_platform_compat_exception = 1022
ui_invoke_explorer_exception = 1023
setting_required_exception = 1024
block_sync_required_exception = 1025



argument_exceed_limit_exception = 2001
argument_size_invalid_exception = 2002
argument_legality_exception = 2003
argument_dismatch_exception = 2004

account_existed_exception = 3001
account_authority_exception = 3002
account_notfound_exception = 3003


account_name_exception = 3201
account_length_exception = 3202
account_address_get_exception = 3203

account_deposit_period_exception = 3301
account_balance_lack_exception = 3302


address_list_empty_exception = 4001
address_list_nullptr_exception = 4002
address_dismatch_account_exception = 4003
address_amount_exception = 4004
address_notfound_exception = 4005
address_generate_exception = 4005


address_invalid_exception = 4010
toaddress_empty_exception = 4011
toaddress_invalid_exception = 4012
toaddress_unrecognized_exception = 4013
fromaddress_empty_exception = 4014
fromaddress_invalid_exception = 4015
fromaddress_unrecognized_exception = 4016

asset_lack_exception = 5001
asset_amount_exception = 5002
asset_notfound_exception = 5003
asset_type_exception = 5004
asset_exchange_poundage_exception = 5005
asset_issue_poundage_exception = 5006
asset_description_length_exception = 5007
asset_symbol_duplicate_exception = 5008
asset_symbol_existed_exception = 5009
asset_symbol_notfound_exception = 5010
asset_symbol_length_exception = 5011
asset_symbol_name_exception = 5012
asset_issued_not_delete = 5013
asset_delete_fail = 5014
asset_secondaryissue_threshold_exception = 5015
asset_attenuation_model_exception = 5016
asset_cert_exception = 5017
asset_cert_existed_exception = 5018
asset_cert_notfound_exception = 5019
asset_cert_notowned_exception = 5020
asset_cert_domain_exception = 5021

etp_lack_exception = 5051

block_height_get_exception = 5101
block_last_height_get_exception = 5102
block_height_exception = 5103
block_hash_get_exception = 5104
block_header_get_exception = 5105

multisig_cosigne_exception = 5201
multisig_exist_exception = 5202
multisig_notfound_exception = 5203
multisig_script_exception = 5204
multisig_index_exception = 5205
signature_amount_exception = 5220
pubkey_amount_exception = 5230
pubkey_dismatch_exception = 5231
prikey_notfound_exception = 5232
pubkey_notfound_exception = 5233

tx_io_exception = 5301
tx_source_exception = 5302
tx_sign_exception = 5303
tx_validate_exception = 5304
tx_broadcast_exception = 5305
tx_notfound_exception = 5306
tx_attachment_value_exception = 5307
tx_fetch_exception = 5308
tx_send_exception = 5309
tx_encode_get_exception = 5310
tx_decode_get_exception = 5311
tx_timestamp_exception = 5312
tx_locktime_exception = 5313

utxo_fetch_exception = 5401

redeem_script_empty_exception = 5501
redeem_script_data_exception = 5502
redeem_script_pattern_exception = 5503


encode_exception = 6001
ec_to_address_exception = 6002
ec_to_public_exception = 6003

did_symbol_name_exception = 7001
did_symbol_existed_exception = 7002
did_symbol_length_exception = 7003
did_description_length_exception = 7004
did_register_poundage_exception = 7005
did_symbol_notfound_exception = 7006
did_symbol_duplicate_exception = 7007
did_address_needed_exception = 7008
did_symbol_notowned_exception = 7009
did_multisig_address_exception = 7010

seed_exception = 9001
seed_size_exception = 9001
seed_length_exception = 9002

hd_length_exception = 9101
hd_key_exception = 9102
hd_new_exception = 9103
hd_private_new_exception = 9104
hd_to_ec_exception = 9105

mnemonicwords_amount_exception = 9201
mnemonicwords_content_exception = 9202
mnemonicwords_new_exception = 9203
mnemonicwords_to_seed_exception = 9204
mnemonicwords_dismatch_exception = 9205
mnemonicwords_empty_exception = 9206
mnemonicwords_existed_exception = 9207