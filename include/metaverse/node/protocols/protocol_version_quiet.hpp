/**
 * Copyright (c) 2011-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2020 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse.
 *
 * metaverse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
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
#ifndef MVS_NODE_PROTOCOL_VERSION_QUIET_HPP
#define MVS_NODE_PROTOCOL_VERSION_QUIET_HPP

#include <memory>
#include <metaverse/network.hpp>
#include <metaverse/node/define.hpp>

namespace libbitcoin {
namespace node {

class BCN_API protocol_version_quiet
  : public network::protocol_version
{
public:
    typedef std::shared_ptr<protocol_version_quiet> ptr;

    /// Construct a quiet version protocol instance.
    protocol_version_quiet(network::p2p& network,
        network::channel::ptr channel);

protected:
    /// Overridden to set relay to false.
    void send_version(const message::version& self) override;
};

} // namespace node
} // namespace libbitcoin

#endif
