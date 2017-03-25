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

#include <metaverse/explorer/commands/fetch-stealth.hpp>

#include <iostream>
#include <metaverse/client.hpp>
#include <metaverse/explorer/callback_state.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/display.hpp>
#include <metaverse/explorer/prop_tree.hpp>


namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::chain;
using namespace bc::client;
using namespace bc::explorer::config;
using namespace bc::wallet;

console_result fetch_stealth::invoke(std::ostream& output, std::ostream& error)
{
    // Bound parameters.
    const auto height = get_height_option();
    const auto& encoding = get_format_option();
    const auto& filter = get_filter_argument();
    const auto connection = get_connection(*this);

    obelisk_client client(connection);

    if (!client.connect(connection))
    {
        display_connection_failure(error, connection.server);
        return console_result::failure;
    }

    if (filter.size() > stealth_address::max_filter_bits)
    {
        error << BX_FETCH_STEALTH_FILTER_TOO_LONG << std::flush;
        return console_result::failure;
    }

    callback_state state(error, output, encoding);

    // This enables json-style array formatting.
    const auto json = encoding == encoding_engine::json;

    auto on_done = [&state, &filter, json](const stealth::list& list)
    {
        // Write out the transaction hashes of *potential* matches.
        state.output(prop_tree(list, json));
    };

    auto on_error = [&state](const std::error_code& error)
    {
        state.succeeded(error);
    };

    client.blockchain_fetch_stealth(on_error, on_done, filter, height);
    client.wait();

    return state.get_result();
}

} //namespace commands 
} //namespace explorer 
} //namespace libbitcoin 
