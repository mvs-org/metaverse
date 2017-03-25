/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS)
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

#ifndef MVS_PROTOCOL_PACKET
#define MVS_PROTOCOL_PACKET

#include <memory>
#include <metaverse/bitcoin.hpp>
#include <metaverse/protocol/define.hpp>
#include <metaverse/protocol/zmq/message.hpp>

namespace libbitcoin {
namespace protocol {

class BCP_API packet
{
public:
    packet();

    const data_chunk origin() const;
    const data_chunk destination() const;
    void set_destination(const data_chunk& destination);

    bool receive(zmq::socket& socket);
    ////bool receive(const std::shared_ptr<zmq::socket>& socket);
    bool send(zmq::socket& socket);
    ////bool send(const std::shared_ptr<zmq::socket>& socket);

protected:
    virtual bool encode_payload(zmq::message& message) const = 0;
    virtual bool decode_payload(const data_chunk& payload) = 0;

private:
    data_chunk origin_;
    data_chunk destination_;
};

}
}

#endif
