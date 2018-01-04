/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
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
#include <metaverse/network/pending_channels.hpp>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/channel.hpp>

namespace libbitcoin {
namespace network {

pending_channels::pending_channels()
{
}

pending_channels::~pending_channels()
{
    BITCOIN_ASSERT_MSG(channels_.empty(), "Pending channels not cleared.");
}

bool pending_channels::safe_store(channel::ptr channel)
{
    const auto version_nonce = channel->nonce();
    const auto match = [version_nonce](const channel::ptr& entry)
    {
        return entry->nonce() == version_nonce;
    };

    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_upgrade();

    const auto it = std::find_if(channels_.begin(), channels_.end(), match);
    const auto found = it != channels_.end();

    if (!found)
    {
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        mutex_.unlock_upgrade_and_lock();
        channels_.push_back(channel);
        mutex_.unlock();
        //---------------------------------------------------------------------
        return true;
    }

    mutex_.unlock_upgrade();
    ///////////////////////////////////////////////////////////////////////////

    return false;
}

void pending_channels::store(channel::ptr channel, result_handler handler)
{
    handler(safe_store(channel) ? error::success : error::address_in_use);
}

bool pending_channels::safe_remove(channel::ptr channel)
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_upgrade();

    const auto it = std::find(channels_.begin(), channels_.end(), channel);
    const auto found = it != channels_.end();

    if (found)
    {
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        mutex_.unlock_upgrade_and_lock();
        channels_.erase(it);
        mutex_.unlock();
        //---------------------------------------------------------------------
        return true;
    }

    mutex_.unlock_upgrade();
    ///////////////////////////////////////////////////////////////////////////

    return false;
}

void pending_channels::remove(channel::ptr channel, result_handler handler)
{
    handler(safe_remove(channel) ? error::success : error::not_found);
}

bool pending_channels::safe_exists(uint64_t version_nonce) const
{
    const auto match = [version_nonce](channel::ptr entry)
    {
        return entry->nonce() == version_nonce;
    };

    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    shared_lock lock(mutex_);

    const auto it = std::find_if(channels_.begin(), channels_.end(), match);
    return it != channels_.end();
    ///////////////////////////////////////////////////////////////////////////
}

void pending_channels::exists(uint64_t version_nonce,
    truth_handler handler) const
{
    // This is an optimization that requires we always set a non-zero nonce.
    handler(version_nonce == 0 ? false : safe_exists(version_nonce));
}

} // namespace network
} // namespace libbitcoin
