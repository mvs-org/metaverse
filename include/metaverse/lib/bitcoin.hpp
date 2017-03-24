///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2015 libbitcoin developers (see COPYING).
//
//        GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY
//
///////////////////////////////////////////////////////////////////////////////
#ifndef MVS_BITCOIN_HPP
#define MVS_BITCOIN_HPP

/**
 * API Users: Include only this header. Direct use of other headers is fragile 
 * and unsupported as header organization is subject to change.
 *
 * Maintainers: Do not include this header internal to this library.
 */

#include <metaverse/lib/bitcoin/compat.h>
#include <metaverse/lib/bitcoin/compat.hpp>
#include <metaverse/lib/bitcoin/constants.hpp>
#include <metaverse/lib/bitcoin/define.hpp>
#include <metaverse/lib/bitcoin/error.hpp>
#include <metaverse/lib/bitcoin/handlers.hpp>
#include <metaverse/lib/bitcoin/messages.hpp>
#include <metaverse/lib/bitcoin/version.hpp>
#include <metaverse/lib/bitcoin/chain/block.hpp>
#include <metaverse/lib/bitcoin/chain/header.hpp>
#include <metaverse/lib/bitcoin/chain/history.hpp>
#include <metaverse/lib/bitcoin/chain/input.hpp>
#include <metaverse/lib/bitcoin/chain/output.hpp>
#include <metaverse/lib/bitcoin/chain/point.hpp>
#include <metaverse/lib/bitcoin/chain/point_iterator.hpp>
#include <metaverse/lib/bitcoin/chain/spend.hpp>
#include <metaverse/lib/bitcoin/chain/stealth.hpp>
#include <metaverse/lib/bitcoin/chain/transaction.hpp>
#include <metaverse/lib/bitcoin/chain/script/opcode.hpp>
#include <metaverse/lib/bitcoin/chain/script/operation.hpp>
#include <metaverse/lib/bitcoin/chain/script/script.hpp>
#include <metaverse/lib/bitcoin/config/authority.hpp>
#include <metaverse/lib/bitcoin/config/base16.hpp>
#include <metaverse/lib/bitcoin/config/base2.hpp>
#include <metaverse/lib/bitcoin/config/base58.hpp>
#include <metaverse/lib/bitcoin/config/base64.hpp>
#include <metaverse/lib/bitcoin/config/checkpoint.hpp>
#include <metaverse/lib/bitcoin/config/directory.hpp>
#include <metaverse/lib/bitcoin/config/endpoint.hpp>
#include <metaverse/lib/bitcoin/config/hash160.hpp>
#include <metaverse/lib/bitcoin/config/hash256.hpp>
#include <metaverse/lib/bitcoin/config/parameter.hpp>
#include <metaverse/lib/bitcoin/config/parser.hpp>
#include <metaverse/lib/bitcoin/config/printer.hpp>
#include <metaverse/lib/bitcoin/config/sodium.hpp>
#include <metaverse/lib/bitcoin/formats/base_10.hpp>
#include <metaverse/lib/bitcoin/formats/base_16.hpp>
#include <metaverse/lib/bitcoin/formats/base_58.hpp>
#include <metaverse/lib/bitcoin/formats/base_64.hpp>
#include <metaverse/lib/bitcoin/formats/base_85.hpp>
#include <metaverse/lib/bitcoin/math/checksum.hpp>
#include <metaverse/lib/bitcoin/math/crypto.hpp>
#include <metaverse/lib/bitcoin/math/elliptic_curve.hpp>
#include <metaverse/lib/bitcoin/math/hash.hpp>
#include <metaverse/lib/bitcoin/math/hash_number.hpp>
#include <metaverse/lib/bitcoin/math/script_number.hpp>
#include <metaverse/lib/bitcoin/math/stealth.hpp>
#include <metaverse/lib/bitcoin/math/uint256.hpp>
#include <metaverse/lib/bitcoin/message/address.hpp>
#include <metaverse/lib/bitcoin/message/alert.hpp>
#include <metaverse/lib/bitcoin/message/alert_payload.hpp>
#include <metaverse/lib/bitcoin/message/block_message.hpp>
#include <metaverse/lib/bitcoin/message/block_transactions.hpp>
#include <metaverse/lib/bitcoin/message/compact_block.hpp>
#include <metaverse/lib/bitcoin/message/fee_filter.hpp>
#include <metaverse/lib/bitcoin/message/filter_add.hpp>
#include <metaverse/lib/bitcoin/message/filter_clear.hpp>
#include <metaverse/lib/bitcoin/message/filter_load.hpp>
#include <metaverse/lib/bitcoin/message/get_address.hpp>
#include <metaverse/lib/bitcoin/message/get_block_transactions.hpp>
#include <metaverse/lib/bitcoin/message/get_blocks.hpp>
#include <metaverse/lib/bitcoin/message/get_data.hpp>
#include <metaverse/lib/bitcoin/message/get_headers.hpp>
#include <metaverse/lib/bitcoin/message/header_message.hpp>
#include <metaverse/lib/bitcoin/message/headers.hpp>
#include <metaverse/lib/bitcoin/message/heading.hpp>
#include <metaverse/lib/bitcoin/message/inventory.hpp>
#include <metaverse/lib/bitcoin/message/inventory_vector.hpp>
#include <metaverse/lib/bitcoin/message/memory_pool.hpp>
#include <metaverse/lib/bitcoin/message/merkle_block.hpp>
#include <metaverse/lib/bitcoin/message/network_address.hpp>
#include <metaverse/lib/bitcoin/message/not_found.hpp>
#include <metaverse/lib/bitcoin/message/ping.hpp>
#include <metaverse/lib/bitcoin/message/pong.hpp>
#include <metaverse/lib/bitcoin/message/prefilled_transaction.hpp>
#include <metaverse/lib/bitcoin/message/reject.hpp>
#include <metaverse/lib/bitcoin/message/send_compact_blocks.hpp>
#include <metaverse/lib/bitcoin/message/send_headers.hpp>
#include <metaverse/lib/bitcoin/message/transaction_message.hpp>
#include <metaverse/lib/bitcoin/message/verack.hpp>
#include <metaverse/lib/bitcoin/message/version.hpp>
#include <metaverse/lib/bitcoin/unicode/console_streambuf.hpp>
#include <metaverse/lib/bitcoin/unicode/ifstream.hpp>
#include <metaverse/lib/bitcoin/unicode/ofstream.hpp>
#include <metaverse/lib/bitcoin/unicode/unicode.hpp>
#include <metaverse/lib/bitcoin/unicode/unicode_istream.hpp>
#include <metaverse/lib/bitcoin/unicode/unicode_ostream.hpp>
#include <metaverse/lib/bitcoin/unicode/unicode_streambuf.hpp>
#include <metaverse/lib/bitcoin/utility/array_slice.hpp>
#include <metaverse/lib/bitcoin/utility/asio.hpp>
#include <metaverse/lib/bitcoin/utility/assert.hpp>
#include <metaverse/lib/bitcoin/utility/atomic.hpp>
#include <metaverse/lib/bitcoin/utility/binary.hpp>
#include <metaverse/lib/bitcoin/utility/collection.hpp>
#include <metaverse/lib/bitcoin/utility/color.hpp>
#include <metaverse/lib/bitcoin/utility/conditional_lock.hpp>
#include <metaverse/lib/bitcoin/utility/container_sink.hpp>
#include <metaverse/lib/bitcoin/utility/container_source.hpp>
#include <metaverse/lib/bitcoin/utility/data.hpp>
#include <metaverse/lib/bitcoin/utility/deadline.hpp>
#include <metaverse/lib/bitcoin/utility/decorator.hpp>
#include <metaverse/lib/bitcoin/utility/delegates.hpp>
#include <metaverse/lib/bitcoin/utility/deserializer.hpp>
#include <metaverse/lib/bitcoin/utility/dispatcher.hpp>
#include <metaverse/lib/bitcoin/utility/enable_shared_from_base.hpp>
#include <metaverse/lib/bitcoin/utility/endian.hpp>
#include <metaverse/lib/bitcoin/utility/exceptions.hpp>
#include <metaverse/lib/bitcoin/utility/istream_reader.hpp>
#include <metaverse/lib/bitcoin/utility/log.hpp>
#include <metaverse/lib/bitcoin/utility/logging.hpp>
#include <metaverse/lib/bitcoin/utility/monitor.hpp>
#include <metaverse/lib/bitcoin/utility/notifier.hpp>
#include <metaverse/lib/bitcoin/utility/ostream_writer.hpp>
#include <metaverse/lib/bitcoin/utility/png.hpp>
#include <metaverse/lib/bitcoin/utility/random.hpp>
#include <metaverse/lib/bitcoin/utility/reader.hpp>
#include <metaverse/lib/bitcoin/utility/resource_lock.hpp>
#include <metaverse/lib/bitcoin/utility/resubscriber.hpp>
#include <metaverse/lib/bitcoin/utility/scope_lock.hpp>
#include <metaverse/lib/bitcoin/utility/serializer.hpp>
#include <metaverse/lib/bitcoin/utility/string.hpp>
#include <metaverse/lib/bitcoin/utility/subscriber.hpp>
#include <metaverse/lib/bitcoin/utility/synchronizer.hpp>
#include <metaverse/lib/bitcoin/utility/thread.hpp>
#include <metaverse/lib/bitcoin/utility/threadpool.hpp>
#include <metaverse/lib/bitcoin/utility/timer.hpp>
#include <metaverse/lib/bitcoin/utility/track.hpp>
#include <metaverse/lib/bitcoin/utility/variable_uint_size.hpp>
#include <metaverse/lib/bitcoin/utility/work.hpp>
#include <metaverse/lib/bitcoin/utility/writer.hpp>
#include <metaverse/lib/bitcoin/wallet/bitcoin_uri.hpp>
#include <metaverse/lib/bitcoin/wallet/dictionary.hpp>
#include <metaverse/lib/bitcoin/wallet/ec_private.hpp>
#include <metaverse/lib/bitcoin/wallet/ec_public.hpp>
#include <metaverse/lib/bitcoin/wallet/ek_private.hpp>
#include <metaverse/lib/bitcoin/wallet/ek_public.hpp>
#include <metaverse/lib/bitcoin/wallet/ek_token.hpp>
#include <metaverse/lib/bitcoin/wallet/encrypted_keys.hpp>
#include <metaverse/lib/bitcoin/wallet/hd_private.hpp>
#include <metaverse/lib/bitcoin/wallet/hd_public.hpp>
#include <metaverse/lib/bitcoin/wallet/message.hpp>
#include <metaverse/lib/bitcoin/wallet/mini_keys.hpp>
#include <metaverse/lib/bitcoin/wallet/mnemonic.hpp>
#include <metaverse/lib/bitcoin/wallet/payment_address.hpp>
#include <metaverse/lib/bitcoin/wallet/qrcode.hpp>
#include <metaverse/lib/bitcoin/wallet/select_outputs.hpp>
#include <metaverse/lib/bitcoin/wallet/settings.hpp>
#include <metaverse/lib/bitcoin/wallet/stealth_address.hpp>
#include <metaverse/lib/bitcoin/wallet/uri.hpp>
#include <metaverse/lib/bitcoin/wallet/uri_reader.hpp>

#endif
