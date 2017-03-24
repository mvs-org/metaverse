/**
 * Copyright (c) 2011-2016 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin-protocol.
 *
 * libbitcoin-protocol is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) 
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MVS_PROTOCOL_ZMQ_AUTHENTICATOR_HPP
#define MVS_PROTOCOL_ZMQ_AUTHENTICATOR_HPP

#include <memory>
#include <functional>
#include <future>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <metaverse/lib/bitcoin.hpp>
#include <metaverse/lib/protocol/define.hpp>
#include <metaverse/lib/protocol/zmq/context.hpp>
#include <metaverse/lib/protocol/zmq/socket.hpp>
#include <metaverse/lib/protocol/zmq/worker.hpp>

namespace libbitcoin {
namespace protocol {
namespace zmq {

/// This class is thread safe.
class BCP_API authenticator
  : public worker
{
public:
    /// A shared authenticator pointer.
    typedef std::shared_ptr<authenticator> ptr;

    /// The fixed inprocess authentication endpoint.
    static const config::endpoint endpoint;

    /// There may be only one authenticator per process.
    authenticator(threadpool& threadpool);

    /// Stop the router.
    virtual ~authenticator();

    /// Expose the authenticated context.
    operator context&();

    /// Start the ZAP router for the context.
    virtual bool start() override;

    /// Stop the router (optional).
    virtual bool stop() override;

    /// This must be called on the socket thread, empty domain allowed.
    /// Set secure false to enable NULL mechanism, otherwise curve is required.
    /// By not applying this method authentication is bypassed altogether.
    /// Apply authentication to the socket for the given arbitrary domain.
    virtual bool apply(socket& socket, const std::string& domain, bool secure);

    /// Set the server private key (required for curve security).
    virtual void set_private_key(const config::sodium& private_key);

    /// Allow clients with the following public keys (whitelist).
    virtual void allow(const hash_digest& public_key);

    /// Allow clients with the following ip addresses (whitelist).
    virtual void allow(const config::authority& address);

    /// Allow clients with the following ip addresses (blacklist).
    virtual void deny(const config::authority& address);

protected:
    void work() override;

private:
    bool allowed_address(const std::string& address) const;
    bool allowed_key(const hash_digest& public_key) const;
    bool allowed_weak(const std::string& domain) const;

    // This is thread safe.
    context context_;

    // These are protected by mutex.
    bool require_address_;
    config::sodium private_key_;
    std::unordered_set<hash_digest> keys_;
    std::unordered_set<std::string> weak_domains_;
    std::unordered_map<std::string, bool> adresses_;
    mutable shared_mutex mutex_;
};

} // namespace zmq
} // namespace protocol
} // namespace libbitcoin

#endif

