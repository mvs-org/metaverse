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
#ifndef MVS_NETWORK_CONNECTIONS_HPP
#define MVS_NETWORK_CONNECTIONS_HPP

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/channel.hpp>
#include <metaverse/network/define.hpp>

namespace libbitcoin {
namespace network {

/// Pool of active connections, thread and lock safe.
class BCT_API connections
  : public enable_shared_from_base<connections>
{
public:
    typedef std::shared_ptr<connections> ptr;
    typedef std::function<void(bool)> truth_handler;
    typedef std::function<void(size_t)> count_handler;
    typedef std::function<void(const code&)> result_handler;
    typedef std::function<void(const code&, channel::ptr)> channel_handler;

    /// Construct an instance.
    connections();

    /// Validate connections stopped.
    ~connections();

    /// This class is not copyable.
    connections(const connections&) = delete;
    void operator=(const connections&) = delete;

    /// Send a message to all channels, with completion handlers.
    /// Complete always returns success, use channel handler for failure codes.
    template <typename Message>
    void broadcast(const Message& message, channel_handler handle_channel,
        result_handler handle_complete)
    {
        // We cannot use a synchronizer here because handler closure in loop.
        auto counter = std::make_shared<std::atomic<size_t>>(channels_.size());

        for (const auto channel: safe_copy())
        {
            const auto handle_send = [=](code ec)
            {
                handle_channel(ec, channel);

                if (counter->fetch_sub(1) == 1)
                    handle_complete(error::success);
            };

            // No pre-serialize, channels may have different protocol versions.
            channel->send(message, handle_send);
        }
    }

    /// Subscribe to all incoming messages of a type.
    template <class Message>
    void subscribe(message_handler<Message>&& handler)
    {
        for (const auto channel: safe_copy())
        {
        	auto handler_copy = handler;
        	channel->subscribe(std::move(handler_copy));//by jianglh
//            channel->subscribe(
//                std::forward<message_handler<Message>>(handler));
        }
    }

    virtual void stop(const code& ec);
    virtual void stop(const config::authority& authority);
    virtual void count(count_handler handler) const;
    virtual void store(channel::ptr channel, result_handler handler);
    virtual void remove(channel::ptr channel, result_handler handler);
    virtual void exists(const config::authority& authority,
        truth_handler handler) const;
    config::authority::list authority_list();

private:
    typedef std::vector<channel::ptr> list;

    list safe_copy() const;
    size_t safe_count() const;
    code safe_store(channel::ptr channel);
    bool safe_remove(channel::ptr channel);
    bool safe_exists(const config::authority& address) const;

    list channels_;
    std::atomic<bool> stopped_;
    mutable upgrade_mutex mutex_;
};

} // namespace network
} // namespace libbitcoin

#endif
