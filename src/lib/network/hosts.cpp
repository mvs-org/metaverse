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
#include <metaverse/network/hosts.hpp>

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>
#include <metaverse/bitcoin.hpp>
#include <metaverse/bitcoin/utility/path.hpp>
#include <metaverse/bitcoin/math/limits.hpp>
#include <metaverse/network/settings.hpp>

namespace libbitcoin {
namespace network {

uint32_t timer_interval = 60 * 5; // 5 minutes


hosts::hosts(threadpool& pool, const settings& settings)
    : seed_count(settings.seeds.size())
    , host_pool_capacity_(std::max(settings.host_pool_capacity, 1u))
    , buffer_(host_pool_capacity_)
    , backup_(host_pool_capacity_)
    , inactive_(host_pool_capacity_ * 2)
    , stopped_(true)
    , file_path_(default_data_path() / settings.hosts_file)
    , disabled_(settings.host_pool_capacity == 0)
    , pool_(pool)
{
}

// private
hosts::iterator hosts::find(const address& host)
{
    return find(buffer_, host);
}

hosts::iterator hosts::find(list& buffer, const address& host)
{
    const auto found = [&host](const address & entry)
    {
        return entry.port == host.port && entry.ip == host.ip;
    };
    return std::find_if(buffer.begin(), buffer.end(), found);
}

size_t hosts::count() const
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    shared_lock lock(mutex_);

    return buffer_.size();
    ///////////////////////////////////////////////////////////////////////////
}

code hosts::fetch(address& out, const config::authority::list& excluded_list)
{
    if (disabled_)
        return error::not_found;

    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    shared_lock lock(mutex_);

    if (stopped_)
        return error::service_stopped;

    if (buffer_.empty())
        return error::not_found;

    // Randomly select an address from the buffer.
    const auto random = pseudo_random(0, buffer_.size() - 1);
    const auto index = static_cast<size_t>(random);
    out = buffer_[index];
    return error::success;
    ///////////////////////////////////////////////////////////////////////////
}

hosts::address::list hosts::copy()
{
    if (disabled_)
        return address::list();

    shared_lock lock{mutex_};

    return address::list(std::begin(buffer_), std::end(buffer_));
}

void hosts::handle_timer(const code& ec)
{
    if (disabled_)
        return;

    if (ec.value() != error::success) {
        return;
    }

    mutex_.lock_upgrade();

    if (stopped_)
    {
        mutex_.unlock_upgrade();
        return;
    }

    mutex_.unlock_upgrade_and_lock();
    bc::ofstream file(file_path_.string());
    const auto file_error = file.bad();

    if (!file_error)
    {
        log::debug(LOG_NETWORK)
                << "sync hosts to file(" << file_path_.string()
                << "), inactive size is " << inactive_.size()
                << ", buffer size is " << buffer_.size();

        for (const auto& entry : buffer_)
        {
            file << config::authority(entry) << std::endl;
        }

        // for (const auto& entry : inactive_)
        //     file << config::authority(entry) << std::endl;
    }
    else
    {
        log::error(LOG_NETWORK) << "hosts file (" << file_path_.string() << ") open failed" ;
        mutex_.unlock();
        return;
    }

    mutex_.unlock();
    snap_timer_->start(std::bind(&hosts::handle_timer, shared_from_this(), std::placeholders::_1));
}

// load
code hosts::start()
{
    if (disabled_)
        return error::success;
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_upgrade();

    if (!stopped_)
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        return error::operation_failed;
    }

    mutex_.unlock_upgrade_and_lock();
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    snap_timer_ = std::make_shared<deadline>(pool_, asio::seconds(timer_interval));
    snap_timer_->start(std::bind(&hosts::handle_timer, shared_from_this(), std::placeholders::_1));

    stopped_ = false;
    bc::ifstream file(file_path_.string());
    const auto file_error = file.bad();

    if (!file_error)
    {
        std::string line;
        while (std::getline(file, line))
        {
            config::authority host(line);

            if (host.port() != 0)
            {
                auto network_address = host.to_network_address();
                if (network_address.is_routable())
                {
                    buffer_.push_back(network_address);
                    if (buffer_.full()) {
                        break;
                    }
                }
                else
                {
                    log::debug(LOG_NETWORK) << "host start is not routable," << config::authority{network_address};
                }
            }
        }
    }

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    if (file_error)
    {
        log::debug(LOG_NETWORK)
                << "Failed to save hosts file.";
        return error::file_system;
    }

    return error::success;
}

// load
code hosts::stop()
{
    if (disabled_)
        return error::success;

    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_upgrade();

    if (stopped_)
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        return error::success;
    }

    mutex_.unlock_upgrade_and_lock();
    
    // stop timer
    snap_timer_->stop();

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    stopped_ = true;
    bc::ofstream file(file_path_.string());
    const auto file_error = file.bad();

    if (!file_error)
    {
        for (const auto& entry : buffer_)
        {
            // TODO: create full space-delimited network_address serialization.
            // Use to/from string format as opposed to wire serialization.
            file << config::authority(entry) << std::endl;
        }

        buffer_.clear();
    }

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    if (file_error)
    {
        log::debug(LOG_NETWORK)
                << "Failed to load hosts file.";
        return error::file_system;
    }

    return error::success;
}

code hosts::clear()
{
    if (disabled_)
        return error::success;

    // Critical Section
    mutex_.lock_upgrade();

    if (stopped_)
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        return error::service_stopped;
    }

    mutex_.unlock_upgrade_and_lock();

    if (!buffer_.empty()) {
        backup_ = std::move( buffer_ );
    }

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    return error::success;
}

code hosts::after_reseeding()
{
    if (disabled_)
        return error::success;

    mutex_.lock_upgrade();

    if (stopped_)
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        return error::service_stopped;
    }

    mutex_.unlock_upgrade_and_lock();

    //re-seeding failed and recover the buffer with backup one
    if (buffer_.size() <= seed_count)
    {
        log::debug(LOG_NETWORK)
                << "Reseeding finished, buffer size: " << buffer_.size()
                << ", less than seed count: " << seed_count
                << ", roll back the hosts cache.";

        if (!buffer_.full())
        {
            for (auto& host : backup_)
            {
                if (find(host) == buffer_.end())
                {
                    buffer_.push_back(host);

                    if (buffer_.full())
                    {
                        break;
                    }
                }
            }
        }
    }
    else {
        // filter inactive hosts
        for (auto &host : inactive_)
        {
            auto iter = find(host);
            if (iter != buffer_.end())
            {
                buffer_.erase(iter);
            }
        }
    }

    // clear the backup
    backup_.clear();

    log::debug(LOG_NETWORK)
            << "Reseeding finished, buffer size: " << buffer_.size();

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    return error::success;
}

code hosts::remove(const address& host)
{
    if (disabled_)
        return error::success;

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_upgrade();

    if (stopped_)
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        return error::service_stopped;
    }

    mutex_.unlock_upgrade_and_lock();

    auto it = find(host);
    if (it != buffer_.end())
    {
        buffer_.erase(it);
    }

    if (find(inactive_, host) == inactive_.end())
    {
        inactive_.push_back(host);
    }

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    return error::success;
}

code hosts::store(const address& host)
{
    if (disabled_)
        return error::success;

    if (!host.is_routable())
    {
        // We don't treat invalid address as an error, just log it.
        return error::success;
    }

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_upgrade();

    if (stopped_)
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        return error::service_stopped;
    }

    mutex_.unlock_upgrade_and_lock();

    if (find(host) == buffer_.end())
    {
        buffer_.push_back(host);
    }

    auto iter = find(inactive_, host);
    if (iter != inactive_.end())
    {
        inactive_.erase(iter);
    }

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    return error::success;
}

// The handler is invoked once all calls to do_store are completed.
// We disperse here to allow other addresses messages to interleave hosts.
void hosts::store(const address::list& hosts, result_handler handler)
{
    if (disabled_ || hosts.empty())
    {
        handler(error::success);
        return;
    }

    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_upgrade();

    if (stopped_)
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        handler(error::service_stopped);
        return;
    }

    // Accept between 1 and all of this peer's addresses up to capacity.
    const auto capacity = host_pool_capacity_;
    size_t host_size = hosts.size();
    const size_t usable = std::min(host_size, capacity);
    const size_t random = static_cast<size_t>(pseudo_random(1, usable));

    // But always accept at least the amount we are short if available.
    const size_t gap = capacity - buffer_.size();
    const size_t accept = std::max(gap, random);

    // Convert minimum desired to step for iteration, no less than 1.
    const auto step = std::max(usable / accept, size_t(1));
    size_t accepted = 0;

    mutex_.unlock_upgrade_and_lock();
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    for (size_t index = 0; index < usable; index = ceiling_add(index, step))
    {
        const auto& host = hosts[index];

        // Do not treat invalid address as an error, just log it.
        if (!host.is_valid())
        {
            log::debug(LOG_NETWORK)
                    << "Invalid host address from peer.";
            continue;
        }

        // Do not allow duplicates in the host cache.
        if (find(host) == buffer_.end()
                && find(inactive_, host) == inactive_.end())
        {
            ++accepted;
            buffer_.push_back(host);
        }
    }

    log::debug(LOG_NETWORK)
            << "Accepted (" << accepted << " of " << hosts.size()
            << ") host addresses from peer."
            << " inactive size is " << inactive_.size()
            << ", buffer size is " << buffer_.size();

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////

    handler(error::success);
}

} // namespace network
} // namespace libbitcoin
