/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
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
#include <bitcoin/network/protocols/protocol.hpp>

#include <string>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/network/channel.hpp>
#include <bitcoin/network/p2p.hpp>

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
