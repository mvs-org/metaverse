/**
 * Copyright (c) 2011-2016 libbitcoin developers (see AUTHORS)
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
#include <metaverse/server/settings.hpp>

#include <metaverse/node.hpp>

namespace libbitcoin {
namespace server {

using namespace asio;

settings::settings()
  : query_workers(1),
    heartbeat_interval_seconds(5),
    subscription_expiration_minutes(10),
    subscription_limit(100000000),
    mongoose_listen("127.0.0.1:8820"),
    administrator_required(false),
    log_level("DEBUG"),
    secure_only(false),
    query_service_enabled(true),
    heartbeat_service_enabled(false),
    block_service_enabled(false),
    transaction_service_enabled(false),
    public_query_endpoint("tcp://*:9091"),
    public_heartbeat_endpoint("tcp://*:9092"),
    public_block_endpoint("tcp://*:9093"),
    public_transaction_endpoint("tcp://*:9094"),
    secure_query_endpoint("tcp://*:9081"),
    secure_heartbeat_endpoint("tcp://*:9082"),
    secure_block_endpoint("tcp://*:9083"),
    secure_transaction_endpoint("tcp://*:9084")
{
}

// There are no current distinctions spanning chain contexts.
settings::settings(bc::settings context)
  : settings()
{
}

duration settings::heartbeat_interval() const
{
    return seconds(heartbeat_interval_seconds);
}

duration settings::subscription_expiration() const
{
    return minutes(subscription_expiration_minutes);
}

} // namespace server
} // namespace libbitcoin
