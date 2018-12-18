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
#include <metaverse/macros_define.hpp>

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>
#include <metaverse/bitcoin.hpp>
#include <metaverse/bitcoin/utility/path.hpp>
#include <metaverse/bitcoin/math/limits.hpp>
#include <metaverse/network/settings.hpp>
#include <metaverse/network/channel.hpp>

namespace libbitcoin {
namespace network {

uint32_t timer_interval = 60 * 5; // 5 minutes


hosts::hosts(threadpool& pool, const settings& settings)
    : seed_count(settings.seeds.size())
    , host_pool_capacity_(std::max(settings.host_pool_capacity, 1u))
    , buffer_(host_pool_capacity_)
    , backup_(host_pool_capacity_)
    , inactive_(host_pool_capacity_ * 2)
    , seeds_()
    , stopped_(true)
    , file_path_(default_data_path() / settings.hosts_file)
    , disabled_(settings.host_pool_capacity == 0)
    , pool_(pool)
    , self_(settings.self)
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

code hosts::fetch_seed(address& out, const config::authority::list& excluded_list)
{
    return fetch(seeds_, out, excluded_list);
}

code hosts::fetch(address& out, const config::authority::list& excluded_list)
{
    return fetch(buffer_, out, excluded_list);
}

template <typename T>
code hosts::fetch(T& buffer, address& out, const config::authority::list& excluded_list)
{
    if (disabled_) {
        return error::not_found;
    }

    // Critical Section
    shared_lock lock(mutex_);

    if (stopped_) {
        return error::service_stopped;
    }

    if (buffer.empty()) {
        return error::not_found;
    }

    auto match = [&excluded_list](address& addr) {
        auto auth = config::authority(addr);
        return std::find(excluded_list.begin(), excluded_list.end(), auth) == excluded_list.end();
    };

    std::vector<address> vec;
    std::copy_if(buffer.begin(), buffer.end(), std::back_inserter(vec), match);

    if (vec.empty()) {
        return error::not_found;
    }

    const auto index = pseudo_random(0, vec.size() - 1);
    out = vec[static_cast<size_t>(index)];

    return error::success;
}

hosts::address::list hosts::copy_seeds()
{
    if (disabled_)
        return address::list();

    shared_lock lock{mutex_};
    return seeds_;
}

hosts::address::list hosts::copy()
{
    if (disabled_)
        return address::list();

    shared_lock lock{mutex_};

    if (stopped_ || buffer_.empty())
        return address::list();

    // not copy all, but just 10% ~ 20% , at least one
    const auto out_count = std::max<size_t>(1,
        std::min<size_t>(1000, buffer_.size()) / pseudo_random(5, 10));

    const auto limit = buffer_.size();
    auto index = pseudo_random(0, limit - 1);

    address::list copy(out_count);

    for (size_t count = 0; count < out_count; ++count)
        copy.push_back(buffer_[index++ % limit]);

    pseudo_random::shuffle(copy);
    return copy;
}

bool hosts::store_cache(bool succeed_clear_buffer)
{
    if (!buffer_.empty()) {
        bc::ofstream file(file_path_.string());
        const auto file_error = file.bad();

        if (file_error) {
            log::error(LOG_NETWORK) << "hosts file (" << file_path_.string() << ") open failed" ;
            return false;
        }

        log::debug(LOG_NETWORK)
                << "sync hosts to file(" << file_path_.string()
                << "), inactive size is " << inactive_.size()
                << ", buffer size is " << buffer_.size();

        for (const auto& entry : buffer_) {
            // TODO: create full space-delimited network_address serialization.
            // Use to/from string format as opposed to wire serialization.
            if (!(channel::blacklisted(entry) || channel::manualbanned(entry))) {
                file << config::authority(entry) << std::endl;
            }
        }

        if (succeed_clear_buffer) {
            buffer_.clear();
        }
    }
    else {
        if (boost::filesystem::exists(file_path_.string())) {
            boost::filesystem::remove_all(file_path_.string());
        }
    }

    return true;
}

void hosts::handle_timer(const code& ec)
{
    if (disabled_) {
        return;
    }

    if (ec.value() != error::success) {
        return;
    }

    // Critical Section
    upgrade_lock lock(mutex_);

    if (stopped_) {
        return;
    }

    {
        upgrade_to_unique_lock unq_lock(lock);

        if (!store_cache()) {
            return;
        }
    }

    snap_timer_->start(std::bind(&hosts::handle_timer, shared_from_this(), std::placeholders::_1));
}

// load
code hosts::start()
{
    if (disabled_) {
        return error::success;
    }

    // Critical Section
    upgrade_lock lock(mutex_);

    if (!stopped_) {
        return error::operation_failed;
    }

    upgrade_to_unique_lock unq_lock(lock);
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    snap_timer_ = std::make_shared<deadline>(pool_, asio::seconds(timer_interval));
    snap_timer_->start(std::bind(&hosts::handle_timer, shared_from_this(), std::placeholders::_1));

    stopped_ = false;

    bc::ifstream file(file_path_.string());
    const auto file_error = file.bad();
    if (!file_error) {
        std::string line;
        while (std::getline(file, line)) {
            config::authority host(line);

            if (host.port() != 0) {
                auto network_address = host.to_network_address();
                if (network_address.is_routable()) {
                    buffer_.push_back(network_address);
                    if (buffer_.full()) {
                        break;
                    }
                }
                else {
                    log::debug(LOG_NETWORK) << "host start is not routable,"
                        << config::authority{network_address};
                }
            }
        }
    }

    if (file_error) {
        log::debug(LOG_NETWORK)
                << "Failed to save hosts file.";
        return error::file_system;
    }

    return error::success;
}

// load
code hosts::stop()
{
    if (disabled_) {
        return error::success;
    }

    // Critical Section
    upgrade_lock lock(mutex_);

    if (stopped_) {
        return error::success;
    }

    upgrade_to_unique_lock unq_lock(lock);

    // stop timer
    snap_timer_->stop();

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    stopped_ = true;

    if (!store_cache(true)) {
        return error::file_system;
    }

    return error::success;
}

code hosts::clear()
{
    if (disabled_) {
        return error::success;
    }

    // Critical Section
    upgrade_lock lock(mutex_);

    if (stopped_) {
        return error::service_stopped;
    }

    upgrade_to_unique_lock unq_lock(lock);

    if (!buffer_.empty()) {
        backup_.clear();
        std::copy(buffer_.begin(), buffer_.end(), std::back_inserter(backup_));
        buffer_.clear();
    }

    return error::success;
}

code hosts::after_reseeding()
{
    if (disabled_) {
        return error::success;
    }

    // Critical Section
    upgrade_lock lock(mutex_);

    if (stopped_) {
        return error::service_stopped;
    }

    upgrade_to_unique_lock unq_lock(lock);

    //re-seeding failed and recover the buffer with backup one
    if (buffer_.size() <= seed_count) {
        log::debug(LOG_NETWORK)
                << "Reseeding finished, buffer size: " << buffer_.size()
                << ", less than seed count: " << seed_count
                << ", roll back the hosts cache.";

        if (!buffer_.full()) {
            for (auto& host : backup_) {
                if (find(host) == buffer_.end()) {
                    buffer_.push_back(host);

                    if (buffer_.full()) {
                        break;
                    }
                }
            }
        }
    }
    else {
        // filter inactive hosts
        for (auto &host : inactive_) {
            auto iter = find(host);
            if (iter != buffer_.end()) {
                buffer_.erase(iter);
            }
        }
    }

    // clear the backup
    backup_.clear();

    log::debug(LOG_NETWORK)
            << "Reseeding finished, buffer size: " << buffer_.size();

    return error::success;
}

code hosts::remove(const address& host)
{
    if (disabled_) {
        return error::success;
    }

    // Critical Section
    upgrade_lock lock(mutex_);

    if (stopped_) {
        return error::service_stopped;
    }

    upgrade_to_unique_lock unq_lock(lock);

    auto it = find(host);
    if (it != buffer_.end()) {
        buffer_.erase(it);
    }

    if (find(inactive_, host) == inactive_.end()) {
        inactive_.push_back(host);
    }

    return error::success;
}

code hosts::store_seed(const address& host)
{
    // don't store blacklist and banned address
    if (channel::blacklisted(host) || channel::manualbanned(host)) {
        return error::success;
    }

    // Critical Section
    upgrade_lock lock(mutex_);

    if (stopped_) {
        return error::service_stopped;
    }

    auto iter = std::find_if(seeds_.begin(), seeds_.end(),
        [&host](const address& item){
            return host.ip == item.ip && host.port == item.port;
        });

    if (iter == seeds_.end()) {
        constexpr size_t max_seeds = 8;
        constexpr size_t half_pos = max_seeds >> 1;
        upgrade_to_unique_lock unq_lock(lock);
        if (seeds_.size() == max_seeds) { // full
            std::copy(seeds_.begin()+half_pos+1, seeds_.end(), seeds_.begin()+half_pos);
            seeds_.back() = host;
        }
        else {
            seeds_.push_back(host);
        }
#ifdef PRIVATE_CHAIN
        log::info(LOG_NETWORK) << "store seed " << config::authority(host).to_string();
#endif
    }

    return error::success;
}

code hosts::remove_seed(const address& host)
{
    if (disabled_) {
        return error::success;
    }

    // Critical Section
    upgrade_lock lock(mutex_);

    if (stopped_) {
        return error::service_stopped;
    }

    auto iter = std::find_if(seeds_.begin(), seeds_.end(),
        [&host](const address& item){
            return host.ip == item.ip && host.port == item.port;
        });

    if (iter != seeds_.end()) {
        upgrade_to_unique_lock unq_lock(lock);
        seeds_.erase(iter);

#ifdef PRIVATE_CHAIN
        log::info(LOG_NETWORK) << "remove seed " << config::authority(host).to_string();
#endif
    }

    return error::success;
}

code hosts::store(const address& host)
{
    if (disabled_) {
        return error::success;
    }

    if (!host.is_routable()) {
        // We don't treat invalid address as an error, just log it.
        return error::success;
    }

    // don't store self address
    auto authority = config::authority{host};
    if (authority == self_ || authority.port() == 0) {
        return error::success;
    }

    // don't store blacklist and banned address
    if (channel::blacklisted(host) || channel::manualbanned(host)) {
        return error::success;
    }

    // Critical Section
    upgrade_lock lock(mutex_);

    if (stopped_) {
        return error::service_stopped;
    }

    upgrade_to_unique_lock unq_lock(lock);

    if (find(host) == buffer_.end()) {
        buffer_.push_back(host);
    }

    auto iter = find(inactive_, host);
    if (iter != inactive_.end()) {
        inactive_.erase(iter);
    }

    return error::success;
}

// The handler is invoked once all calls to do_store are completed.
// We disperse here to allow other addresses messages to interleave hosts.
void hosts::store(const address::list& hosts, result_handler handler)
{
    if (disabled_ || hosts.empty()) {
        handler(error::success);
        return;
    }

    // Critical Section
    upgrade_lock lock(mutex_);

    if (stopped_) {
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

    {
        upgrade_to_unique_lock unq_lock(lock);
        //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

        for (size_t index = 0; index < usable; index = ceiling_add(index, step)) {
            const auto& host = hosts[index];

            // Do not treat invalid address as an error, just log it.
            if (!host.is_valid()) {
                log::debug(LOG_NETWORK)
                        << "Invalid host address from peer.";
                continue;
            }

            if (channel::blacklisted(host) || channel::manualbanned(host)) {
                continue;
            }

            // Do not allow duplicates in the host cache.
            if (find(host) == buffer_.end()
                    && find(inactive_, host) == inactive_.end()) {
                ++accepted;
                buffer_.push_back(host);
            }
        }

        log::debug(LOG_NETWORK)
                << "Accepted (" << accepted << " of " << hosts.size()
                << ") host addresses from peer."
                << " inactive size is " << inactive_.size()
                << ", buffer size is " << buffer_.size();
    }

    // Notice: don't unique lock this handler
    handler(error::success);
}

} // namespace network
} // namespace libbitcoin
