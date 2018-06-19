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
#include <metaverse/protocol/zmq/zeromq.hpp>

#include <zmq.h>
#include <metaverse/bitcoin.hpp>

namespace libbitcoin {
namespace protocol {
namespace zmq {

// See: zmq::errno_to_string
code get_last_error()
{
    switch (zmq_errno())
    {
        case 0:
            return error::success;

#if defined _WIN32
        case ENOBUFS:
        case ENOTSUP:
        case EPROTONOSUPPORT:
            return error::operation_failed;
        case ENETDOWN:
            return error::network_unreachable;
        case EADDRINUSE:
            return error::address_in_use;
        case EADDRNOTAVAIL:
            return error::resolve_failed;
        case ECONNREFUSED:
            return error::accept_failed;
        case EINPROGRESS:
            return error::channel_timeout;
#endif
        case EFSM:
        case EAGAIN:
            return error::channel_timeout;
        case EFAULT:
            return error::bad_stream;
        case EINTR:
        case ETERM:
            return error::service_stopped;
        case ENOTSOCK:
        case EMTHREAD:
        case ENOCOMPATPROTO:
            return error::operation_failed;
        default:
            return error::unknown;
    }
}

} // namespace zmq
} // namespace protocol
} // namespace libbitcoin
