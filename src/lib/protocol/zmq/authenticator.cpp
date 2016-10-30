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
#include <bitcoin/protocol/zmq/authenticator.hpp>

#include <functional>
#include <future>
#include <string>
#include <zmq.h>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/protocol/zmq/message.hpp>
#include <bitcoin/protocol/zmq/context.hpp>
#include <bitcoin/protocol/zmq/poller.hpp>
#include <bitcoin/protocol/zmq/socket.hpp>
#include <bitcoin/protocol/zmq/worker.hpp>

namespace libbitcoin {
namespace protocol {
namespace zmq {

// ZAP endpoint, see: rfc.zeromq.org/spec:27/ZAP
const config::endpoint authenticator::endpoint("inproc://zeromq.zap.01");

// There may be only one authenticator per process.
authenticator::authenticator(threadpool& pool)
  : worker(pool),
    context_(false),
    require_address_(false)
{
}

authenticator::~authenticator()
{
    stop();
}

authenticator::operator context&()
{
    return context_;
}

// Restartable after stop and not started on construct.
bool authenticator::start()
{
    // Context is thread safe, this critical section is for start atomicity.
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    unique_lock lock(mutex_);

    return context_.start() && worker::start();
    ///////////////////////////////////////////////////////////////////////////
}

bool authenticator::stop()
{
    // Context is thread safe, this critical section is for stop atomicity.
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    unique_lock lock(mutex_);

    // Stop the context first in case a blocking proxy is in use.
    return context_.stop() && worker::stop();
    ///////////////////////////////////////////////////////////////////////////
}

// github.com/zeromq/rfc/blob/master/src/spec_27.c
void authenticator::work()
{
    socket router(context_, zmq::socket::role::router);

    if (!started(router.bind(endpoint) == error::success))
        return;

    poller poller;
    poller.add(router);

    while (!poller.terminated() && !stopped())
    {
        if (!poller.wait().contains(router.id()))
            continue;

        data_chunk origin;
        data_chunk delimiter;
        std::string version;
        std::string sequence;
        std::string status_code;
        std::string status_text;
        std::string userid;
        std::string metadata;

        message request;
        auto ec = router.receive(request);

        if (ec != error::success || request.size() < 8)
        {
            status_code = "500";
            status_text = "Internal error.";
        }
        else
        {
            origin = request.dequeue_data();
            delimiter = request.dequeue_data();
            version = request.dequeue_text();
            sequence = request.dequeue_text();
            const auto domain = request.dequeue_text();
            const auto address = request.dequeue_text();
            const auto identity = request.dequeue_text();
            const auto mechanism = request.dequeue_text();

            // ZAP authentication should not occur with an empty domain.
            if (origin.empty() || !delimiter.empty() || version != "1.0" ||
                sequence.empty() || domain.empty() || !identity.empty())
            {
                status_code = "500";
                status_text = "Internal error.";
            }
            else if (!allowed_address(address))
            {
                // Address restrictions are independent of mechanisms.
                status_code = "400";
                status_text = "Address not enabled for access.";
            }
            else
            {
                if (mechanism == "NULL")
                {
                    if (request.size() != 0)
                    {
                        status_code = "400";
                        status_text = "Incorrect NULL parameterization.";
                    }
                    else if (!allowed_weak(domain))
                    {
                        status_code = "400";
                        status_text = "NULL mechanism not authorized.";
                    }
                    else
                    {
                        // It is more efficient to use an unsecured context or
                        // to not start the authenticator, but this works too.
                        status_code = "200";
                        status_text = "OK";
                        userid = "anonymous";
                    }
                }
                else if (mechanism == "CURVE")
                {
                    if (request.size() != 1)
                    {
                        status_code = "400";
                        status_text = "Incorrect CURVE parameterization.";
                    }
                    else
                    {
                        hash_digest public_key;

                        if (!request.dequeue(public_key))
                        {
                            status_code = "400";
                            status_text = "Invalid public key.";
                        }
                        else if (!allowed_key(public_key))
                        {
                            status_code = "400";
                            status_text = "Public key not authorized.";
                        }
                        else
                        {
                            status_code = "200";
                            status_text = "OK";
                            userid = "unspecified";
                        }
                    }
                }
                else if (mechanism == "PLAIN")
                {
                    if (request.size() != 2)
                    {
                        status_code = "400";
                        status_text = "Incorrect PLAIN parameterization.";
                    }
                    else
                    {
                        ////userid = request.dequeue_text();
                        ////const auto password = request.dequeue_text();
                        status_code = "400";
                        status_text = "PLAIN mechanism not supported.";
                    }
                }
                else
                {
                    status_code = "400";
                    status_text = "Security mechanism not supported.";
                }
            }
        }

        message response;
        response.enqueue(origin);
        response.enqueue(delimiter);
        response.enqueue(version);
        response.enqueue(sequence);
        response.enqueue(status_code);
        response.enqueue(status_text);
        response.enqueue(userid);
        response.enqueue(metadata);

        DEBUG_ONLY(ec =) router.send(response);
        BITCOIN_ASSERT_MSG(!ec, "Failed to send ZAP response.");
    }

    finished(router.stop());
}

// This must be called on the socket thread.
// Addresses and client keys may be updated after this is applied.
// The configuration at the time of this call determines the mode of security.
bool authenticator::apply(socket& socket, const std::string& domain,
    bool secure)
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_shared();
    const auto private_key = private_key_;
    const auto have_public_keys = !keys_.empty();
    const auto require_address = require_address_;
    mutex_.unlock_shared();
    ///////////////////////////////////////////////////////////////////////////

    // A private server key is required if there are public client keys.
    if (have_public_keys && !private_key)
        return false;

    if (!secure)
    {
        if (require_address)
        {
            // These persist after a socket closes so don't reuse domain names.
            weak_domains_.emplace(domain);
            return socket.set_authentication_domain(domain);
        }

        // There are no address or curve rules to apply so bypass ZAP.
        return true;
    }

    if (private_key)
    {
        return
            socket.set_private_key(private_key) &&
            socket.set_curve_server() &&
            socket.set_authentication_domain(domain);
    }

    // We do not have a private key to set so we cannot set secure.
    return false;
}

void authenticator::set_private_key(const config::sodium& private_key)
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    unique_lock lock(mutex_);

    private_key_ = private_key;
    ///////////////////////////////////////////////////////////////////////////
}

bool authenticator::allowed_address(const std::string& ip_address) const
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    shared_lock lock(mutex_);

    return !require_address_ || adresses_.find(ip_address) != adresses_.end();
    ///////////////////////////////////////////////////////////////////////////
}

bool authenticator::allowed_key(const hash_digest& public_key) const
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    shared_lock lock(mutex_);

    return keys_.empty() || keys_.find(public_key) != keys_.end();
    ///////////////////////////////////////////////////////////////////////////
}

bool authenticator::allowed_weak(const std::string& domain) const
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    shared_lock lock(mutex_);

    return weak_domains_.find(domain) != weak_domains_.end();
    ///////////////////////////////////////////////////////////////////////////
}

void authenticator::allow(const hash_digest& public_key)
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    unique_lock lock(mutex_);

    keys_.emplace(public_key);
    ///////////////////////////////////////////////////////////////////////////
}

void authenticator::allow(const config::authority& address)
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    unique_lock lock(mutex_);

    require_address_ = true;
    adresses_.emplace(address.to_hostname(), true);
    ///////////////////////////////////////////////////////////////////////////
}

void authenticator::deny(const config::authority& address)
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    unique_lock lock(mutex_);

    // Denial is effective independent of whitelisting.
    adresses_.emplace(address.to_hostname(), false);
    ///////////////////////////////////////////////////////////////////////////
}

} // namespace zmq
} // namespace protocol
} // namespace libbitcoin
