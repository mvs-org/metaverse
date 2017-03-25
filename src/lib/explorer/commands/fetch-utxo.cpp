/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse-explorer.
 *
 * metaverse-explorer is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <metaverse/explorer/commands/fetch-utxo.hpp>

#include <iostream>
#include <metaverse/client.hpp>
#include <metaverse/explorer/callback_state.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/display.hpp>
#include <metaverse/explorer/prop_tree.hpp>
#include <metaverse/explorer/config/algorithm.hpp>


namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::client;
using namespace bc::explorer::config;

console_result fetch_utxo::invoke(std::ostream& output, std::ostream& error)
{
    // Bound parameters.
    const auto& encoding = get_format_option();
    const auto algorithm = get_algorithm_option();
    const auto satoshi = get_satoshi_argument();
    const auto& address = get_payment_address_argument();
    const auto connection = get_connection(*this);

    obelisk_client client(connection);

    if (!client.connect(connection))
    {
        display_connection_failure(error, connection.server);
        return console_result::failure;
    }

    callback_state state(error, output, encoding);

    // This enables json-style array formatting.
    const auto json = encoding == encoding_engine::json;

    auto on_done = [&state, json](const bc::chain::points_info& selected_utxos)
    {
        state.output(prop_tree(selected_utxos, json));
    };

    auto on_error = [&state](const code& error)
    {
        state.succeeded(error);
    };

    client.address_fetch_unspent_outputs(
        on_error, on_done, address, satoshi, algorithm);
    client.wait();

    return state.get_result();
}

} //namespace commands 
} //namespace explorer 
} //namespace libbitcoin 
