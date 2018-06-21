/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifdef MVS_VERSION4

#include <metaverse/protocol/packet.hpp>

#include <memory>
#include <metaverse/bitcoin.hpp>
#include <metaverse/protocol/interface.pb.h>
#include <metaverse/protocol/response_packet.hpp>
#include <metaverse/protocol/zmq/message.hpp>
#include <metaverse/protocol/zmq/socket.hpp>

namespace libbitcoin {
namespace protocol {

response_packet::response_packet()
  : response_(nullptr)
{
}

std::shared_ptr<response> response_packet::get_response() const
{
    return response_;
}

void response_packet::set_response(std::shared_ptr<response> payload)
{
    response_ = payload;
}

bool response_packet::encode_payload(zmq::message& message) const
{
    if (!response_)
        return false;

    const auto data = response_->SerializeAsString();
    message.append({ data.begin(), data.end() });
    return true;
}

bool response_packet::decode_payload(const data_chunk& payload)
{
    const auto data = std::make_shared<response>();

    if (!data->ParseFromString({ payload.begin(), payload.end() }))
        return false;

    response_ = data;
    return true;
}

}
}

#endif
