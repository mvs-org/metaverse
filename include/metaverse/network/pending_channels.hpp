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
#ifndef MVS_NETWORK_PENDING_CHANNELS_HPP
#define MVS_NETWORK_PENDING_CHANNELS_HPP

#include <cstdint>
#include <functional>
#include <vector>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/channel.hpp>
#include <metaverse/network/define.hpp>

namespace libbitcoin {
namespace network {

/// Class to manage a pending channel pool, thread and lock safe.
class BCT_API pending_channels
{
public:
    typedef std::function<void(bool)> truth_handler;
    typedef std::function<void(const code&)> result_handler;

    pending_channels();
    ~pending_channels();

    /// This class is not copyable.
    pending_channels(const pending_channels&) = delete;
    void operator=(const pending_channels&) = delete;

    virtual void store(channel::ptr channel, result_handler handler);
    virtual void remove(channel::ptr channel, result_handler handler);
    virtual void exists(uint64_t version_nonce, truth_handler handler) const;

private:
    typedef std::vector<channel::ptr> list;

    bool safe_store(channel::ptr channel);
    bool safe_remove(channel::ptr channel);
    bool safe_exists(uint64_t version_nonce) const;

    list channels_;
    mutable upgrade_mutex mutex_;
};

} // namespace network
} // namespace libbitcoin

#endif

