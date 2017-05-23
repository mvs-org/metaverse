/**
 * Copyright (c) 2011-2016 libbitcoin developers (see AUTHORS)
 *
 * This file is part of metaverse-protocol.
 *
 * metaverse-protocol is free software: you can redistribute it and/or
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
#include <metaverse/protocol/zmq/context.hpp>

#include <cstdint>
#include <zmq.h>
#include <metaverse/bitcoin.hpp>
#include <mutex>

namespace libbitcoin {
namespace protocol {
namespace zmq {

std::mutex zmq_mtx;
static constexpr int32_t zmq_fail = -1;

context::context(bool started)
  : self_(nullptr)
{
    if (started)
        start();
}

context::~context()
{
    stop();
}

// Restartable after stop and optionally started on construct.
bool context::start()
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    std::unique_lock<std::mutex> zmq_lock(zmq_mtx);
    unique_lock lock(mutex_);

    if (self_ != nullptr)
        return false;

    self_ = zmq_ctx_new();
    return self_ != nullptr;
    ///////////////////////////////////////////////////////////////////////////
}

// Signal termination and block until all sockets closed.
bool context::stop()
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    std::unique_lock<std::mutex> zmq_lock(zmq_mtx);
    unique_lock lock(mutex_);

    if (self_ == nullptr)
        return true;

    // This aborts blocking operations but blocks here until either each socket
    // in the context is explicitly closed. This can fail by signal interrupt.
    auto self = self_;
    self_ = nullptr;
    return zmq_ctx_term(self) != zmq_fail;
    ///////////////////////////////////////////////////////////////////////////
}

context::operator const bool() const
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    shared_lock lock(mutex_);

    return self_ != nullptr;
    ///////////////////////////////////////////////////////////////////////////
}

void* context::self()
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    shared_lock lock(mutex_);

    // This may become invalid after return. The guard only ensures atomicity.
    return self_;
    ///////////////////////////////////////////////////////////////////////////
}

} // namespace zmq
} // namespace protocol
} // namespace libbitcoin
