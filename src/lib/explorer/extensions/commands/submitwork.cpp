/**
 * Copyright (c) 2019-2020 mvs developers
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


#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/submitwork.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ submitwork *************************/
inline bool startswith(const string &str, const char *prefix) {
    return str.find(prefix) == 0;
}

console_result submitwork::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& miner = node.miner();

    const uint64_t nounce_mask = (get_api_version() == 3) ? 0 : 0x6675636b6d657461;
    //Note: submitwork does not starts with 0x, while eth_submitWork does!
    if (get_api_version() == 3) {
        if ( !startswith(argument_.nonce, "0x") ) {
            throw argument_legality_exception{"nonce should start with \"0x\" for eth_submitWork"};
        }
        argument_.nonce = argument_.nonce.substr(2, argument_.nonce.size()-2);
    }

    auto ret = miner.put_result(argument_.nonce, argument_.mix_hash, argument_.header_hash, nounce_mask);
    auto& root = jv_output;

    if (ret) {
        if (get_api_version() == 1) {
            root["result"] = "true"; // boost json parser output as string, for compatible.
        }
        else {
            root = true;
        }
    }
    else {
        if (get_api_version() == 1) {
            root["result"] = "false"; // boost json parser output as string, for compatible.
        }
        else {
            root = false;
        }
    }

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

