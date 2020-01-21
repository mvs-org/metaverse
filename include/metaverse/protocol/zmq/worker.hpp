/*
 * Copyright (c) 2019-2020 libbitcoin developers (see AUTHORS)
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
#ifndef MVS_PROTOCOL_ZMQ_WORKER_HPP
#define MVS_PROTOCOL_ZMQ_WORKER_HPP

#include <atomic>
#include <memory>
#include <future>
#include <metaverse/bitcoin.hpp>
#include <metaverse/protocol/define.hpp>
#include <metaverse/protocol/zmq/socket.hpp>

namespace libbitcoin {
namespace protocol {
namespace zmq {

/// This class is thread safe.
class BCP_API worker
{
public:
    /// A shared worker pointer.
    typedef std::shared_ptr<worker> ptr;

    /// Construct a worker.
    worker(threadpool& pool);

    /// This class is not copyable.
    worker(const worker&) = delete;
    void operator=(const worker&) = delete;

    /// Stop the worker.
    virtual ~worker();

    /// Start the worker.
    virtual bool start();

    /// Stop the worker (optional).
    virtual bool stop();

protected:
    bool stopped();
    bool started(bool result);
    bool finished(bool result);
    bool forward(socket& from, socket& to);
    void relay(socket& left, socket& right);

    virtual void work() = 0;

private:

    // These are protected by mutex.
    dispatcher dispatch_;
    std::atomic<bool> stopped_;
    std::promise<bool> started_;
    std::promise<bool> finished_;
    mutable shared_mutex mutex_;
};

} // namespace zmq
} // namespace protocol
} // namespace libbitcoin

#endif
