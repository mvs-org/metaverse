/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS)
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
#include <metaverse/network/settings.hpp>

namespace libbitcoin {
namespace network {

#define NAME "hosts"

hosts::hosts(threadpool& pool, const settings& settings)
  : buffer_(std::max(settings.host_pool_capacity, 1u)),
    stopped_(true),
    dispatch_(pool, NAME),
    file_path_(default_data_path() / settings.hosts_file),
    disabled_(settings.host_pool_capacity == 0),
	pool_{pool}
{
}

// private
hosts::iterator hosts::find(const address& host)
{
    const auto found = [&host](const address& entry)
    {
        return entry.port == host.port && entry.ip == host.ip;
    };

    return std::find_if(buffer_.begin(), buffer_.end(), found);
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
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
//    shared_lock lock(mutex_);
	mutex_.lock_upgrade();

    if (stopped_)
    {
    	mutex_.unlock_upgrade();
        return error::service_stopped;
    }

    mutex_.unlock_upgrade_and_lock();
    if (buffer_.empty())
    {
    	mutex_.unlock();
        return error::not_found;
    }

    // Randomly select an address from the buffer.

    config::authority::list addresses;

    for(auto entry: buffer_)
    {
		auto iter = std::find(excluded_list.begin(), excluded_list.end(), config::authority(entry) );
		if(iter == excluded_list.end())
		{
			addresses.push_back(config::authority(entry));
		}
    }
    mutex_.unlock();

    if(addresses.empty()){
    	return error::not_satisfied;
    }
    const auto index = static_cast<size_t>(pseudo_random() % addresses.size());
    out = addresses[index].to_network_address();
    return error::success;
    ///////////////////////////////////////////////////////////////////////////
}

hosts::address::list hosts::copy()
{
	address::list copy;

	shared_lock lock{mutex_};
	copy.reserve(buffer_.size());
	std::find_if(buffer_.begin(), buffer_.end(), [&copy](const address& addr){
		copy.push_back(addr);
		return false;
	});
	return copy;
}

void hosts::handle_timer(const code& ec)
{
	if (ec.value() != error::success){
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
		log::debug(LOG_NETWORK) << "sync hosts to file(" << file_path_.string() << "), " << buffer_.size() << " hosts found";
		for (const auto& entry: buffer_)
			file << config::authority(entry) << std::endl;
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
    snap_timer_ = std::make_shared<deadline>(pool_, asio::seconds(60));
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
            	if(network_address.is_routable())
				{
					buffer_.push_back(network_address);
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

    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_upgrade();

    if (stopped_)
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        return error::success;
    }

    mutex_.unlock_upgrade_and_lock();
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    snap_timer_->stop();
    stopped_ = true;
    bc::ofstream file(file_path_.string());
    const auto file_error = file.bad();

    if (!file_error)
    {
        for (const auto& entry: buffer_)
            file << config::authority(entry) << std::endl;

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

code hosts::remove(const address& host)
{
    ///////////////////////////////////////////////////////////////////////////
    // Critical Section
    mutex_.lock_upgrade();

    if (stopped_)
    {
        mutex_.unlock_upgrade();
        //---------------------------------------------------------------------
        return error::service_stopped;
    }

    auto it = find(host);

    if (it != buffer_.end())
    {
        mutex_.unlock_upgrade_and_lock();
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        buffer_.erase(it);

        mutex_.unlock();
        //---------------------------------------------------------------------
        return error::success;
    }

    mutex_.unlock_upgrade();
    ///////////////////////////////////////////////////////////////////////////

    return error::not_found;
}

code hosts::store(const address& host)
{
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

    if (find(host) == buffer_.end())
    {
        mutex_.unlock_upgrade_and_lock();
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        buffer_.push_back(host);

        mutex_.unlock();
        //---------------------------------------------------------------------
        return error::success;
    }

    mutex_.unlock_upgrade();
    ///////////////////////////////////////////////////////////////////////////

//    log::trace(LOG_NETWORK)
//        << "Redundant host address from peer";

    // We don't treat redundant address as an error, just log it.
    return error::success;
}

// private
void hosts::do_store(const address& host, result_handler handler)
{
    handler(store(host));
}

// The handler is invoked once all calls to do_store are completed.
// We disperse here to allow other addresses messages to interleave hosts.
void hosts::store(const address::list& hosts, result_handler handler)
{
    if (stopped_)
        return;

    dispatch_.parallel(hosts, "hosts", handler,
        &hosts::do_store, shared_from_this());
}

} // namespace network
} // namespace libbitcoin
