/**
 * Copyright (c) 2019-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2019-2020 metaverse core developers (see MVS-AUTHORS)
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
#include <metaverse/network/protocols/protocol.hpp>

#include <string>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/channel.hpp>
#include <metaverse/network/p2p.hpp>

namespace libbitcoin {
namespace network {

protocol::protocol(p2p& network, channel::ptr channel,
    const std::string& name)
  : pool_(network.thread_pool()),
    channel_(channel),
    name_(name)
{
}

config::authority protocol::authority() const
{
    return channel_->authority();
}

const std::string& protocol::name() const
{
    return name_;
}

uint64_t protocol::nonce() const
{
    return channel_->nonce();
}

message::version protocol::peer_version() const
{
    return channel_->version();
}

void protocol::set_peer_version(message::version::ptr value)
{
    channel_->set_version(value);
}

uint32_t protocol::peer_start_height()
{
    return channel_->peer_start_height();
}

threadpool& protocol::pool()
{
    return pool_;
}

// Stop the channel.
void protocol::stop(const code& ec)
{
    channel_->stop(ec);
}

bool protocol::misbehaving(int32_t howmuch)
{
    return channel_->misbehaving(howmuch);
}

} // namespace network
} // namespace libbitcoin
