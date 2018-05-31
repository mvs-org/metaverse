/**
 * Copyright (c) 2016-2018 mvs developers
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


#include <jsoncpp/json/json.h>
#include <metaverse/client.hpp>
#include <metaverse/explorer/callback_state.hpp>
#include <metaverse/explorer/extensions/commands/getblockheader.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/json_helper.hpp>
#include <metaverse/explorer/display.hpp>
#include <metaverse/explorer/utility.hpp>
#include <metaverse/explorer/define.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::client;
using namespace bc::explorer::config;

/************************ getblockheader *************************/

console_result getblockheader::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{

    uint64_t height = 0;
    auto& blockchain = node.chain_impl();
    if(!blockchain.get_last_height(height))
        throw block_last_height_get_exception{"query last height failure."};

    if (option_.height != std::numeric_limits<uint32_t>::max()) {
        height = option_.height;
    }

    const auto connection = get_connection(*this);

    obelisk_client client(connection);

    if (!client.connect(connection))
    {
        throw connection_exception{"Could not connect to mvsd port 9921."};
    }
    encoding json_format{"json"};
    std::ostringstream output;
    callback_state state(output, output, json_format);

    auto on_done = [this, &jv_output](const chain::header& header)
    {
        auto&& jheader = config::json_helper(get_api_version()).prop_tree(header);

    	if( !jheader.isObject()
    	    || !jheader["result"].isObject()
    	    || !jheader["result"]["hash"].isString()) {

        	throw block_hash_get_exception{"getbestblockhash got parser exception."};
	    }

        if (option_.is_getbestblockhash) {
    	    auto&& blockhash = jheader["result"]["hash"].asString();
            jv_output = blockhash;
        } else {
            if (get_api_version() == 1) {
                jv_output = jheader;
            } else {
                jv_output = jheader["result"];
            }
        }
    };

    auto on_error = [&state](const code& error)
    {
        state.succeeded(error);
    };

    // Height is ignored if both are specified.
    // Use the null_hash as sentinel to determine whether to use height or hash.
    const hash_digest& hash = option_.hash;
    if (hash == null_hash)
        client.blockchain_fetch_block_header(on_error, on_done, height);
    else
        client.blockchain_fetch_block_header(on_error, on_done, hash);

    client.wait();

    return state.get_result();
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

