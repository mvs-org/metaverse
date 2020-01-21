///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019-2020 libbitcoin developers (see COPYING).
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

#include <metaverse/bitcoin/compat.h>
#include <metaverse/bitcoin/compat.hpp>
#include <metaverse/bitcoin/constants.hpp>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/error.hpp>
#include <metaverse/bitcoin/handlers.hpp>
#include <metaverse/bitcoin/messages.hpp>
#include <metaverse/bitcoin/version.hpp>
#include <metaverse/bitcoin/chain/block.hpp>
#include <metaverse/bitcoin/chain/header.hpp>
#include <metaverse/bitcoin/chain/history.hpp>
#include <metaverse/bitcoin/chain/input.hpp>
#include <metaverse/bitcoin/chain/output.hpp>
#include <metaverse/bitcoin/chain/point.hpp>
#include <metaverse/bitcoin/chain/point_iterator.hpp>
#include <metaverse/bitcoin/chain/spend.hpp>
#include <metaverse/bitcoin/chain/stealth.hpp>
#include <metaverse/bitcoin/chain/transaction.hpp>
#include <metaverse/bitcoin/chain/script/opcode.hpp>
#include <metaverse/bitcoin/chain/script/operation.hpp>
#include <metaverse/bitcoin/chain/script/script.hpp>
#include <metaverse/bitcoin/config/authority.hpp>
#include <metaverse/bitcoin/config/base16.hpp>
#include <metaverse/bitcoin/config/base2.hpp>
#include <metaverse/bitcoin/config/base58.hpp>
#include <metaverse/bitcoin/config/base64.hpp>
#include <metaverse/bitcoin/config/checkpoint.hpp>
#include <metaverse/bitcoin/config/directory.hpp>
#include <metaverse/bitcoin/config/endpoint.hpp>
#include <metaverse/bitcoin/config/hash160.hpp>
#include <metaverse/bitcoin/config/hash256.hpp>
#include <metaverse/bitcoin/config/parameter.hpp>
#include <metaverse/bitcoin/config/parser.hpp>
#include <metaverse/bitcoin/config/printer.hpp>
#include <metaverse/bitcoin/config/sodium.hpp>
#include <metaverse/bitcoin/formats/base_10.hpp>
#include <metaverse/bitcoin/formats/base_16.hpp>
#include <metaverse/bitcoin/formats/base_58.hpp>
#include <metaverse/bitcoin/formats/base_64.hpp>
#include <metaverse/bitcoin/formats/base_85.hpp>
#include <metaverse/bitcoin/math/checksum.hpp>
#include <metaverse/bitcoin/math/crypto.hpp>
#include <metaverse/bitcoin/math/elliptic_curve.hpp>
#include <metaverse/bitcoin/math/hash.hpp>
#include <metaverse/bitcoin/math/hash_number.hpp>
#include <metaverse/bitcoin/math/script_number.hpp>
#include <metaverse/bitcoin/math/stealth.hpp>
#include <metaverse/bitcoin/math/uint256.hpp>
#include <metaverse/bitcoin/message/address.hpp>
#include <metaverse/bitcoin/message/block_message.hpp>
#include <metaverse/bitcoin/message/block_transactions.hpp>
#include <metaverse/bitcoin/message/compact_block.hpp>
#include <metaverse/bitcoin/message/fee_filter.hpp>
#include <metaverse/bitcoin/message/filter_add.hpp>
#include <metaverse/bitcoin/message/filter_clear.hpp>
#include <metaverse/bitcoin/message/filter_load.hpp>
#include <metaverse/bitcoin/message/get_address.hpp>
#include <metaverse/bitcoin/message/get_block_transactions.hpp>
#include <metaverse/bitcoin/message/get_blocks.hpp>
#include <metaverse/bitcoin/message/get_data.hpp>
#include <metaverse/bitcoin/message/get_headers.hpp>
#include <metaverse/bitcoin/message/header_message.hpp>
#include <metaverse/bitcoin/message/headers.hpp>
#include <metaverse/bitcoin/message/heading.hpp>
#include <metaverse/bitcoin/message/inventory.hpp>
#include <metaverse/bitcoin/message/inventory_vector.hpp>
#include <metaverse/bitcoin/message/memory_pool.hpp>
#include <metaverse/bitcoin/message/merkle_block.hpp>
#include <metaverse/bitcoin/message/network_address.hpp>
#include <metaverse/bitcoin/message/not_found.hpp>
#include <metaverse/bitcoin/message/ping.hpp>
#include <metaverse/bitcoin/message/pong.hpp>
#include <metaverse/bitcoin/message/prefilled_transaction.hpp>
#include <metaverse/bitcoin/message/reject.hpp>
#include <metaverse/bitcoin/message/send_compact_blocks.hpp>
#include <metaverse/bitcoin/message/send_headers.hpp>
#include <metaverse/bitcoin/message/transaction_message.hpp>
#include <metaverse/bitcoin/message/verack.hpp>
#include <metaverse/bitcoin/message/version.hpp>
#include <metaverse/bitcoin/unicode/console_streambuf.hpp>
#include <metaverse/bitcoin/unicode/ifstream.hpp>
#include <metaverse/bitcoin/unicode/ofstream.hpp>
#include <metaverse/bitcoin/unicode/unicode.hpp>
#include <metaverse/bitcoin/unicode/unicode_istream.hpp>
#include <metaverse/bitcoin/unicode/unicode_ostream.hpp>
#include <metaverse/bitcoin/unicode/unicode_streambuf.hpp>
#include <metaverse/bitcoin/utility/array_slice.hpp>
#include <metaverse/bitcoin/utility/asio.hpp>
#include <metaverse/bitcoin/utility/assert.hpp>
#include <metaverse/bitcoin/utility/atomic.hpp>
#include <metaverse/bitcoin/utility/binary.hpp>
#include <metaverse/bitcoin/utility/collection.hpp>
#include <metaverse/bitcoin/utility/color.hpp>
#include <metaverse/bitcoin/utility/conditional_lock.hpp>
#include <metaverse/bitcoin/utility/container_sink.hpp>
#include <metaverse/bitcoin/utility/container_source.hpp>
#include <metaverse/bitcoin/utility/data.hpp>
#include <metaverse/bitcoin/utility/deadline.hpp>
#include <metaverse/bitcoin/utility/decorator.hpp>
#include <metaverse/bitcoin/utility/delegates.hpp>
#include <metaverse/bitcoin/utility/deserializer.hpp>
#include <metaverse/bitcoin/utility/dispatcher.hpp>
#include <metaverse/bitcoin/utility/enable_shared_from_base.hpp>
#include <metaverse/bitcoin/utility/endian.hpp>
#include <metaverse/bitcoin/utility/exceptions.hpp>
#include <metaverse/bitcoin/utility/istream_reader.hpp>
#include <metaverse/bitcoin/utility/log.hpp>
#include <metaverse/bitcoin/utility/logging.hpp>
#include <metaverse/bitcoin/utility/monitor.hpp>
#include <metaverse/bitcoin/utility/notifier.hpp>
#include <metaverse/bitcoin/utility/ostream_writer.hpp>
#include <metaverse/bitcoin/utility/png.hpp>
#include <metaverse/bitcoin/utility/random.hpp>
#include <metaverse/bitcoin/utility/reader.hpp>
#include <metaverse/bitcoin/utility/resource_lock.hpp>
#include <metaverse/bitcoin/utility/resubscriber.hpp>
#include <metaverse/bitcoin/utility/scope_lock.hpp>
#include <metaverse/bitcoin/utility/serializer.hpp>
#include <metaverse/bitcoin/utility/string.hpp>
#include <metaverse/bitcoin/utility/subscriber.hpp>
#include <metaverse/bitcoin/utility/synchronizer.hpp>
#include <metaverse/bitcoin/utility/thread.hpp>
#include <metaverse/bitcoin/utility/threadpool.hpp>
#include <metaverse/bitcoin/utility/timer.hpp>
#include <metaverse/bitcoin/utility/track.hpp>
#include <metaverse/bitcoin/utility/variable_uint_size.hpp>
#include <metaverse/bitcoin/utility/work.hpp>
#include <metaverse/bitcoin/utility/writer.hpp>
#include <metaverse/bitcoin/wallet/bitcoin_uri.hpp>
#include <metaverse/bitcoin/wallet/dictionary.hpp>
#include <metaverse/bitcoin/wallet/ec_private.hpp>
#include <metaverse/bitcoin/wallet/ec_public.hpp>
#include <metaverse/bitcoin/wallet/ek_private.hpp>
#include <metaverse/bitcoin/wallet/ek_public.hpp>
#include <metaverse/bitcoin/wallet/ek_token.hpp>
#include <metaverse/bitcoin/wallet/encrypted_keys.hpp>
#include <metaverse/bitcoin/wallet/hd_private.hpp>
#include <metaverse/bitcoin/wallet/hd_public.hpp>
#include <metaverse/bitcoin/wallet/message.hpp>
#include <metaverse/bitcoin/wallet/mini_keys.hpp>
#include <metaverse/bitcoin/wallet/mnemonic.hpp>
#include <metaverse/bitcoin/wallet/payment_address.hpp>
#include <metaverse/bitcoin/wallet/qrcode.hpp>
#include <metaverse/bitcoin/wallet/select_outputs.hpp>
#include <metaverse/bitcoin/wallet/settings.hpp>
#include <metaverse/bitcoin/wallet/stealth_address.hpp>
#include <metaverse/bitcoin/wallet/uri.hpp>
#include <metaverse/bitcoin/wallet/uri_reader.hpp>

#endif
