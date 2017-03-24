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
#ifndef MVS_PROTOCOL_ZMQ_CONTEXT_HPP
#define MVS_PROTOCOL_ZMQ_CONTEXT_HPP

#include <cstdint>
#include <memory>
#include <metaverse/lib/bitcoin.hpp>
#include <metaverse/lib/protocol/define.hpp>

namespace libbitcoin {
namespace protocol {
namespace zmq {

/// This class is thread safe.
class BCP_API context
  : public enable_shared_from_base<context>
{
public:
    /// A shared context pointer.
    typedef std::shared_ptr<context> ptr;

    /// Construct a context.
    context(bool started=true);

    /// This class is not copyable.
    context(const context&) = delete;
    void operator=(const context&) = delete;

    /// Blocks until all child sockets are closed.
    /// Stops all child socket activity by closing the zeromq context.
    virtual ~context();

    /// True if the context is valid and started.
    operator const bool() const;

    /// The underlying zeromq context.
    void* self();

    /// Create the zeromq context.
    virtual bool start();

    /// Blocks until all child sockets are closed.
    /// Stops all child socket activity by closing the zeromq context.
    virtual bool stop();

private:

    // The context pointer is protected by mutex.
    void* self_;
    mutable shared_mutex mutex_;
};

} // namespace zmq
} // namespace protocol
} // namespace libbitcoin

#endif

