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
#include <metaverse/network/protocols/protocol_address.hpp>

#include <functional>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/channel.hpp>
#include <metaverse/network/p2p.hpp>
#include <metaverse/network/protocols/protocol.hpp>
#include <metaverse/network/protocols/protocol_events.hpp>

namespace libbitcoin {
namespace network {

#define NAME "address"
#define CLASS protocol_address

using namespace bc::message;
using namespace std::placeholders;

protocol_address::protocol_address(p2p& network, channel::ptr channel)
  : protocol_events(network, channel, NAME),
    network_(network),
    CONSTRUCT_TRACK(protocol_address)
{
}

protocol_address::ptr protocol_address::do_subscribe()
{
    SUBSCRIBE2(address, handle_receive_address, _1, _2);
    SUBSCRIBE2(get_address, handle_receive_get_address, _1, _2);
    // Must have a handler to capture a shared self pointer in stop subscriber.
	protocol_events::start(BIND1(handle_stop, _1));
    return std::dynamic_pointer_cast<protocol_address>(protocol::shared_from_this());
}

// Start sequence.
// ----------------------------------------------------------------------------

void protocol_address::start()
{
    const auto& settings = network_.network_settings();

    if (settings.self.port() != 0)
    {
		network_address nt_address=settings.self.to_network_address();

		//for testnet don't filter local ip
		if (settings.hosts_file == "hosts-test.cache") {
			self_ = address({ { nt_address } });
			SEND2(self_, handle_send, _1, self_.command);
		}
		//only outer address can be broadcast
		else if (!nt_address.is_private_network()) {
			self_ = address({ { nt_address } });
			SEND2(self_, handle_send, _1, self_.command);
		}
        
    }

#ifdef USE_UPNP
	config::authority out_address = network_.get_out_address();

	if (settings.upnp_map_port && settings.be_found && settings.self != out_address) {
		network_address nt_address = out_address.to_network_address();
		if (settings.hosts_file == "hosts-test.cache") {
			address self = address({ { nt_address } });
			log::info("UPnP") << "send addresss " << out_address.to_string();
			SEND2(self, handle_send, _1, self.command);
		}
		else if (!nt_address.is_private_network()) {
			address self = address({ { nt_address } });
			log::info("UPnP") << "send addresss " << out_address.to_string();
			SEND2(self, handle_send, _1, self.command);
		}
	}
#endif
	

    // If we can't store addresses we don't ask for or handle them.
    if (settings.host_pool_capacity == 0)
        return;

    SEND2(get_address(), handle_send, _1, get_address::command);
}

void protocol_address::remove_useless_address(address::ptr& message)
{
    auto& addresses = message->addresses;
    const auto& settings = network_.network_settings();
    if(settings.self.port() != 0)
    {
        auto iter = std::find_if(addresses.begin(), addresses.end(), [&settings](const message::network_address& addr){
            if(config::authority{addr} == settings.self)
            {
                return true;
            }
            return false;
        });

        if(iter != addresses.end())
        {
            addresses.erase(iter);
        }
    }
}

// Protocol.
// ----------------------------------------------------------------------------

bool protocol_address::handle_receive_address(const code& ec,
    address::ptr message)
{
    if (stopped())
        return false;

    if (ec)
    {
        log::trace(LOG_NETWORK)
            << "Failure receiving address message from ["
            << authority() << "] " << ec.message();
      	stop(ec);

        return false;
    }
    remove_useless_address(message);
    log::trace(LOG_NETWORK)
        << "Storing addresses from [" << authority() << "] ("
        << message->addresses.size() << ")";

//    if (message->addresses.size() > 1000)
//    {
//        return ! misbehaving(20);
//    }
    network_address::list addresses;
    addresses.reserve(message->addresses.size());
    for (auto& addr:message->addresses) {
        if (!channel::blacklisted(addr)) {
            addresses.push_back(addr);
        }
    }

    // TODO: manage timestamps (active channels are connected < 3 hours ago).
    network_.store(message->addresses, BIND2(handle_store_addresses, _1, message));

    // RESUBSCRIBE
    return true;
}

bool protocol_address::handle_receive_get_address(const code& ec,
    get_address::ptr message)
{
    if (stopped())
        return false;

    if (ec)
    {
        log::trace(LOG_NETWORK)
            << "Failure receiving get_address message from ["
            << authority() << "] " << ec.message();
       	stop(ec);
        return false;
    }


    // TODO: allowing repeated queries can allow a channel to map our history.
    // TODO: pull active hosts from host cache (currently just resending self).
    // TODO: need to distort for privacy, don't send currently-connected peers.

    auto&& address_list = network_.address_list();
    auto channel_authorithy = authority();

    auto iter = std::find_if(address_list.begin(), address_list.end(), [&channel_authorithy](const message::network_address& address){
        if(config::authority{address} == channel_authorithy)
        {
            return true;
        }
        return false;
    });
    if(iter != address_list.end() )
    {
        address_list.erase(iter);
    }

    if(address_list.empty())
    {
        return true;
    }
//    if (self_.addresses.empty())
//        return false;

    log::trace(LOG_NETWORK)
        << "Sending addresses to [" << authority() << "] ("
        << address_list.size() << ")";
    message::address self_address = {address_list};
    SEND2(self_address, handle_send, _1, self_address.command);

    // RESUBSCRIBE
    return true;
}

void protocol_address::handle_store_addresses(const code& ec, address::ptr message)
{
    if (stopped())
        return;

    if (ec)
    {
        log::error(LOG_NETWORK)
            << "Failure storing addresses from [" << authority() << "] "
            << ec.message();
        stop(ec);
    }
}

void protocol_address::handle_stop(const code&)
{
    log::trace(LOG_NETWORK)
        << "Stopped addresss protocol";
}

} // namespace network
} // namespace libbitcoin
