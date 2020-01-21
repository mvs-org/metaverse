/**
 * Copyright (c) 2019-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2019-2020 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse-client.
 *
 * metaverse-client is free software: you can redistribute it and/or
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
#ifndef MVS_CLIENT_DEALER_HPP
#define MVS_CLIENT_DEALER_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <metaverse/bitcoin.hpp>
#include <metaverse/client/define.hpp>
#include <metaverse/client/stream.hpp>

namespace libbitcoin {
namespace client {

// This class is not thread safe.
/// Matches replies and outgoing messages, accounting for timeouts and retries.
// This class is a pure proxy; it does not talk directly to zeromq.
class BCC_API dealer
  : public stream
{
public:
    typedef std::function<void(const code&)> error_handler;
    typedef std::function<void(const std::string&)> unknown_handler;
    typedef std::function<void(const binary&, size_t, const hash_digest&,
        const chain::transaction&)> stealth_update_handler;
    typedef std::function<void(const wallet::payment_address&, size_t,
        const hash_digest&, const chain::transaction&)> update_handler;

    /// Resend is unrelated to connections.
    /// Timeout is capped at max_int32 (vs. max_uint32).
    dealer(stream& out, unknown_handler on_unknown_command,
        uint32_t timeout_milliseconds, uint8_t resends);

    virtual ~dealer();

    /// True if there are no outstanding requests.
    bool empty() const;

    /// Clear all handlers with the specified error code.
    void clear(const code& code);

    /// Accessors.
    virtual void set_on_update(update_handler on_update);
    virtual void set_on_stealth_update(stealth_update_handler on_update);

    // stream interface.
    //-------------------------------------------------------------------------

    /// Resend any timed out work and return the smallest time remaining.
    virtual int32_t refresh() override;

    /// Read from this stream onto the specified stream.
    virtual bool read(stream& stream) override;

    /// Write the specified data to this stream.
    virtual bool write(const data_stack& data) override;

protected:
    // Decodes a message and calls the appropriate callback.
    typedef std::function<bool(reader& payload)> decoder;

    struct obelisk_message
    {
        std::string command;
        uint32_t id;
        data_chunk payload;
    };

    struct pending_request
    {
        obelisk_message message;
        error_handler on_error;
        decoder on_reply;
        uint32_t resends;
        asio::time_point deadline;
    };

    // Calculate the number of milliseconds remaining in the deadline.
    static int32_t remaining(const asio::time_point& deadline);

    // send_request->send
    bool send(const obelisk_message& message);

    // write->receive->decode_reply
    bool receive(const obelisk_message& message);

    // Sends an outgoing request, and adds handlers to pending request table.
    bool send_request(const std::string& command, const data_chunk& payload,
        error_handler on_error, decoder on_reply);

    // Decodes an incoming message, invoking the error and/or reply handler.
    // The reply handler must not invoke its handler if there is an error.
    void decode_reply(const obelisk_message& message, error_handler& on_error,
        decoder& on_reply);

    // Payment address notification update.
    void decode_payment_update(const obelisk_message& message);

    // Stealth address notification update.
    void decode_stealth_update(const obelisk_message& message);

    uint32_t last_request_index_;
    const uint8_t resends_;
    const int32_t timeout_milliseconds_;
    const unknown_handler on_unknown_;
    update_handler on_update_;
    stealth_update_handler on_stealth_update_;
    std::map<uint32_t, pending_request> pending_;
    stream& out_;
};

} // namespace client
} // namespace libbitcoin

#endif

