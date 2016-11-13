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
#ifndef LIBBITCOIN_NETWORK_HOSTS_HPP
#define LIBBITCOIN_NETWORK_HOSTS_HPP

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/network/define.hpp>
#include <bitcoin/network/settings.hpp>

namespace libbitcoin {
namespace network {

/// This class is thread safe.
/// The hosts class manages a thread-safe dynamic store of network addresses.
/// The store can be loaded and saved from/to the specified file path.
/// The file is a line-oriented set of config::authority serializations.
/// Duplicate addresses and those with zero-valued ports are disacarded.
class BCT_API hosts
  : public enable_shared_from_base<hosts>
{
public:
    typedef std::shared_ptr<hosts> ptr;
    typedef message::network_address address;
    typedef std::function<void(const code&)> result_handler;

    /// Construct an instance.
    hosts(threadpool& pool, const settings& settings);

    /// This class is not copyable.
    hosts(const hosts&) = delete;
    void operator=(const hosts&) = delete;

    /// Load hosts file if found.
    virtual code start();

    // Save hosts to file.
    virtual code stop();

    virtual size_t count() const;
    virtual code fetch(address& out, const config::authority::list& excluded_list);
    virtual code remove(const address& host);
    virtual code store(const address& host);
    virtual void store(const address::list& hosts, result_handler handler);
    address::list copy();
private:
    typedef boost::circular_buffer<address> list;
    typedef list::iterator iterator;

    iterator find(const address& host);
    void do_store(const address& host, result_handler handler);

    // These are protected by a mutex.
    list buffer_;
    std::atomic<bool> stopped_;
    mutable upgrade_mutex mutex_;

    // This is thread safe.
    dispatcher dispatch_;

    // HACK: we use this because the buffer capacity cannot be set to zero.
    const bool disabled_;
    const boost::filesystem::path file_path_;
};

} // namespace network
} // namespace libbitcoin

#endif

