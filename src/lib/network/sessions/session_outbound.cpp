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
#include <metaverse/network/sessions/session_outbound.hpp>

#include <cstddef>
#include <functional>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/p2p.hpp>
#include <metaverse/network/protocols/protocol_address.hpp>
#include <metaverse/network/protocols/protocol_ping.hpp>
#include <metaverse/bitcoin/utility/deadline.hpp>

namespace libbitcoin {
namespace network {

#define CLASS session_outbound

using namespace std::placeholders;

session_outbound::session_outbound(p2p& network)
  : session_batch(network, true),
    network__(network),
    CONSTRUCT_TRACK(session_outbound)
{
    outbound_counter = 0;
    in_reseeding = false;
}

session_outbound::~session_outbound()
{
    if (reseeding_timer_) {
        reseeding_timer_->stop();
    }
    for (auto connect_timer : connect_timer_list_) {
        connect_timer->stop();
    }
    connect_timer_list_.clear();
}

// Start sequence.
// ----------------------------------------------------------------------------

void session_outbound::start(result_handler handler)
{
    if (settings_.outbound_connections == 0)
    {
        log::info(LOG_NETWORK)
            << "Not configured for generating outbound connections.";
        handler(error::success);
        return;
    }

    session::start(CONCURRENT2(handle_started, _1, handler));
}

void session_outbound::handle_started(const code& ec, result_handler handler)
{
    if (ec)
    {
        handler(ec);
        return;
    }

    outbound_counter = 0;
    in_reseeding = false;

    reseeding_timer_ = std::make_shared<deadline>(pool_, asio::seconds(60));

    const auto connect = create_connector();

    auto self = shared_from_this();
    auto make_timer = [this, self]() -> deadline::ptr {
        connect_timer_list_.emplace_back(std::make_shared<deadline>(pool_, asio::seconds(2)));
        return connect_timer_list_.back();
    };

    for (auto i = 0; i < 3; ++i) {
        new_connection(connect, make_timer(), true, true); // connect seed first
    }

    while (outbound_counter == 0 && !stopped()) { // loop until connected successfully
        new_connection(connect, nullptr, false, false);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    for (size_t peer = 0; peer < settings_.outbound_connections; ++peer) {
        new_connection(connect, make_timer(), true, false);
    }

    // This is the end of the start sequence.
    handler(error::success);
}

// Connnect cycle.
// ----------------------------------------------------------------------------

void session_outbound::new_connection(
    connector::ptr connect, deadline::ptr connect_timer, bool reconnect, bool only_seed)
{
    if (stopped())
    {
        log::trace(LOG_NETWORK)
            << "Suspended outbound connection.";
        return;
    }
    if (only_seed) {
        this->connect_seed(connect, BIND6(handle_connect, _1, _2, connect, connect_timer, reconnect, only_seed));
    }
    else {
        this->connect(connect, BIND6(handle_connect, _1, _2, connect, connect_timer, reconnect, only_seed));
    }
}

void session_outbound::delay_new_connect(connector::ptr connect, deadline::ptr connect_timer, bool only_seed)
{
    if (!connect_timer) {
        return;
    }
    auto self = shared_from_this();
    connect_timer->start([this, self, connect, connect_timer, only_seed](const code& ec){
        if (ec || stopped()) {
            return;
        }
        pool_.service().post(
            std::bind(&session_outbound::new_connection,
                shared_from_base<session_outbound>(),
                connect, connect_timer, true, only_seed));
    });
}

void session_outbound::delay_reseeding()
{
    if (!network__.network_settings().enable_re_seeding) {
        return;
    }

    if (in_reseeding) {
        return;
    }
    in_reseeding = true;
    log::debug(LOG_NETWORK) << "outbound channel counter decreased, the re-seeding will be triggered 60s later!";
    auto self = shared_from_this();
    reseeding_timer_->start([this, self](const code& ec){
        if (ec || stopped()) {
            in_reseeding = false;
            return;
        }
        pool_.service().post(
            std::bind(&session_outbound::handle_reseeding,
                shared_from_base<session_outbound>()));
    });
}

void session_outbound::handle_reseeding()
{
    const int counter = outbound_counter;
    if (counter > 1) {
        log::debug(LOG_NETWORK)
            << "outbound channel counter recovered to ["
            << counter
            << "], re-seeding is canceled!";
    }
    else {
        log::debug(LOG_NETWORK) << "start re-seeding!";
        network__.restart_seeding();
    }
    in_reseeding = false;
}

void session_outbound::handle_connect(const code& ec, channel::ptr channel,
    connector::ptr connect, deadline::ptr connect_timer, bool reconnect, bool only_seed)
{
    if (channel && blacklisted(channel->authority())) {
        log::trace(LOG_NETWORK)
            << "Suspended blacklisted/banned outbound connection [" << channel->authority() << "]";
        return;
    }

    if (ec)
    {
        log::trace(LOG_NETWORK)
            << "Failure connecting outbound: " << ec.message();
        if (reconnect) {
            delay_new_connect(connect, connect_timer, only_seed);
        }
        return;
    }

    log::trace(LOG_NETWORK)
        << "Connected to outbound channel [" << channel->authority() << "]";
    ++outbound_counter;

    register_channel(channel,
        BIND3(handle_channel_start, _1, connect, channel),
        BIND5(handle_channel_stop, _1, connect, channel, connect_timer, only_seed));
}

void session_outbound::handle_channel_start(const code& ec,
    connector::ptr connect, channel::ptr channel)
{
    // Treat a start failure just like a stop.
    if (ec)
    {
        log::trace(LOG_NETWORK)
            << "Outbound channel failed to start ["
            << channel->authority() << "] " << ec.message();
        channel->invoke_protocol_start_handler(error::channel_stopped);
        channel->stop(ec);
        return;
    }

    attach_protocols(channel);

    network__.store_seed(channel->authority().to_network_address(), [](const code&){});
};

void session_outbound::attach_protocols(channel::ptr channel)
{
    attach<protocol_ping>(channel)->do_subscribe()->start();
    attach<protocol_address>(channel)->do_subscribe()->start();
}

void session_outbound::handle_channel_stop(
    const code& ec, connector::ptr connect, channel::ptr channel,
    deadline::ptr connect_timer, bool only_seed)
{
    channel->invoke_protocol_start_handler(error::channel_stopped);
    log::trace(LOG_NETWORK) << "channel stopped," << ec.message();

    const int counter = --outbound_counter;

    if (stopped(ec)) {
        return;
    }

    delay_new_connect(connect, connect_timer, only_seed);
    //restart the seeding procedure with in 1 minutes when outbound session count reduce to 1.
    if (counter <= 1) {
        delay_reseeding();
    }
}



} // namespace network
} // namespace libbitcoin
