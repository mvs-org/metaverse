/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin-client.
 *
 * libbitcoin-client is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
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
#ifndef LIBBITCOIN_CLIENT_PROXY_HPP
#define LIBBITCOIN_CLIENT_PROXY_HPP

#include <functional>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/client/dealer.hpp>
#include <bitcoin/client/define.hpp>
#include <bitcoin/client/stream.hpp>

namespace libbitcoin {
namespace client {

/// Decodes and encodes messages in the obelisk protocol.
/// This class is a pure proxy; it does not talk directly to zeromq.
class BCC_API proxy
  : public dealer
{
public:
    /// Resend is unrelated to connections.
    /// Timeout is capped at max_int32 (vs. max_uint32).
    proxy(stream& out, unknown_handler on_unknown_command,
        uint32_t timeout_milliseconds, uint8_t resends);

    // Fetch handler types.
    //-------------------------------------------------------------------------

    typedef std::function<void()> empty_handler;
    typedef std::function<void(size_t)> height_handler;
    typedef std::function<void(size_t, size_t)> transaction_index_handler;
    typedef std::function<void(const chain::header&)> block_header_handler;
    typedef std::function<void(const chain::history::list&)> history_handler;
    typedef std::function<void(const chain::stealth::list&)> stealth_handler;
    typedef std::function<void(const chain::transaction&)> transaction_handler;
    typedef std::function<void(const chain::point::indexes&)> validate_handler;
    typedef std::function<void(const chain::points_info&)> points_info_handler;


    // Fetchers.
    //-------------------------------------------------------------------------

    void protocol_broadcast_transaction(error_handler on_error,
        empty_handler on_reply, const chain::transaction& tx);

    void transaction_pool_validate(error_handler on_error,
        validate_handler on_reply, const chain::transaction& tx);

    void transaction_pool_fetch_transaction(error_handler on_error,
        transaction_handler on_reply, const hash_digest& tx_hash);

    void blockchain_fetch_transaction(error_handler on_error,
        transaction_handler on_reply, const hash_digest& tx_hash);

    void blockchain_fetch_last_height(error_handler on_error,
        height_handler on_reply);

    void blockchain_fetch_block_header(error_handler on_error,
        block_header_handler on_reply, uint32_t height);

    void blockchain_fetch_block_header(error_handler on_error,
        block_header_handler on_reply, const hash_digest& block_hash);

    void blockchain_fetch_transaction_index(error_handler on_error,
        transaction_index_handler on_reply, const hash_digest& tx_hash);

    void blockchain_fetch_stealth(error_handler on_error,
        stealth_handler on_reply, const binary& prefix,
        uint32_t from_height=0);

    void blockchain_fetch_history(error_handler on_error,
        history_handler on_reply, const wallet::payment_address& address,
        uint32_t from_height = 0);

    /// sx and bs 2.0 only (obsolete in bs 3.0).
    void address_fetch_history(error_handler on_error,
        history_handler on_reply, const wallet::payment_address& address,
        uint32_t from_height=0);

    /// bs 2.0 and later.
    void address_fetch_history2(error_handler on_error,
        history_handler on_reply, const wallet::payment_address& address,
        uint32_t from_height=0);

    void address_fetch_unspent_outputs(error_handler on_error,
        points_info_handler on_reply, const wallet::payment_address& address,
        const uint64_t satoshi,
        const wallet::select_outputs::algorithm algorithm);

    // Subscribers.
    //-------------------------------------------------------------------------

    void address_subscribe(error_handler on_error, empty_handler on_reply,
        const wallet::payment_address& address);

    void address_subscribe(error_handler on_error, empty_handler on_reply,
        chain::subscribe_type type, const binary& prefix);

private:

    // Response handlers.
    //-------------------------------------------------------------------------
    static bool decode_empty(reader& payload, empty_handler& handler);
    static bool decode_transaction(reader& payload,
        transaction_handler& handler);
    static bool decode_height(reader& payload, height_handler& handler);
    static bool decode_block_header(reader& payload,
        block_header_handler& handler);
    static bool decode_transaction_index(reader& payload,
        transaction_index_handler& handler);
    static bool decode_validate(reader& payload, validate_handler& handler);
    static bool decode_stealth(reader& payload, stealth_handler& handler);
    static bool decode_history(reader& payload, history_handler& handler);
    static bool decode_expanded_history(reader& payload,
        history_handler& handler);

    // Utilities.
    //-------------------------------------------------------------------------
    static chain::stealth::list expand(chain::stealth_compact::list& compact);
    static chain::history::list expand(chain::history_compact::list& compact);
};

} // namespace client
} // namespace libbitcoin

#endif
