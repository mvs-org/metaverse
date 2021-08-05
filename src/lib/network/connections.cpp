/**
 * Copyright (c) 2011-2021 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2021 metaverse core developers (see MVS-AUTHORS)
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
#include <metaverse/network/connections.hpp>

#include <algorithm>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/channel.hpp>

namespace libbitcoin {
namespace network {

using namespace bc::config;

#define NAME "connections"

connections::connections()
  : stopped_(false)
{
}

connections::~connections()
{
    BITCOIN_ASSERT_MSG(channels_.empty(), "Connections was not cleared.");
}

// This is idempotent.
void connections::stop(const code& ec)
{
    connections::list channels;

    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_upgrade();

    if (!stopped_)
    {
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        mutex_.unlock_upgrade_and_lock();
        stopped_ = true;
        mutex_.unlock_and_lock_upgrade();
        //---------------------------------------------------------------------

        // Once stopped this list cannot change, but must copy to escape lock.
        channels = channels_;
    }

    mutex_.unlock_upgrade();
    ///////////////////////////////////////////////////////////////////////////

    // Channel stop handlers should remove channels from list.
    for (const auto& channel: channels)
        channel->stop(ec);
}

connections::list connections::safe_copy() const
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    shared_lock lock(mutex_);

    return channels_;
    ///////////////////////////////////////////////////////////////////////////
}

bool connections::safe_exists(const authority& address) const
{
    const auto match = [&address](channel::ptr entry)
    {
        return entry->authority() == address;
    };

    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    shared_lock lock(mutex_);

    const auto it = std::find_if(channels_.begin(), channels_.end(), match);
    return it != channels_.end();
    ///////////////////////////////////////////////////////////////////////////
}

void connections::exists(const authority& address, truth_handler handler) const
{
    handler(safe_exists(address));
}

config::authority::list connections::authority_list()
{
    config::authority::list address_list;
    address_list.reserve(channels_.size());
    mutex_.lock_upgrade();
    std::find_if(channels_.begin(), channels_.end(), [&address_list](channel::ptr channel){
        address_list.push_back(channel->authority());
        return false;
    });
    mutex_.unlock_upgrade();
    return address_list;
}

bool connections::safe_remove(channel::ptr channel)
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

void connections::remove(channel::ptr channel, result_handler handler)
{
    handler(safe_remove(channel) ? error::success : error::not_found);
}

void connections::stop(const config::authority& address)
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    shared_lock lock(mutex_);

    for(auto it = channels_.begin(); it != channels_.end(); ++it)
    {
        if ( (*it)->authority().ip() == address.ip())
        {
            (*it)->stop(error::address_blocked);
        }
    }
}

code connections::safe_store(channel::ptr channel)
{
    const auto nonce = channel->nonce();
    const auto& authority = channel->authority();
    const auto match = [&authority, nonce](channel::ptr entry)
    {
        return entry->authority() == authority || entry->nonce() == nonce;
    };

    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_upgrade();

    const auto stopped = stopped_.load();

    if (!stopped)
    {
        auto it = std::find_if(channels_.begin(), channels_.end(), match);
        const auto found = it != channels_.end();

        if (!found)
        {
            //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            mutex_.unlock_upgrade_and_lock();
            channels_.push_back(channel);
            mutex_.unlock();
            //-----------------------------------------------------------------
            return error::success;
        }
    }

    mutex_.unlock_upgrade();
    ///////////////////////////////////////////////////////////////////////////

    // Stopped and found are the only ways to get here.
    return stopped ? error::service_stopped : error::address_in_use;
}

void connections::store(channel::ptr channel, result_handler handler)
{
    handler(safe_store(channel));
}

size_t connections::safe_count() const
{
    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    shared_lock lock(mutex_);

    return channels_.size();
    ///////////////////////////////////////////////////////////////////////////
}

void connections::count(count_handler handler) const
{
    handler(safe_count());
}

} // namespace network
} // namespace libbitcoin
