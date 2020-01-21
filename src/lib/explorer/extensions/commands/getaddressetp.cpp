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

#include <jsoncpp/json/json.h>
#include <metaverse/explorer/extensions/base_helper.hpp>
#include <metaverse/explorer/extensions/commands/getaddressetp.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ getaddressetp *************************/

console_result getaddressetp::invoke(Json::Value& jv_output,
                                     libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto& addr = argument_.address;
    bc::explorer::commands::balances addr_balance{0, 0, 0, 0};

    sync_fetchbalance(addr, blockchain, addr_balance);

    Json::Value jv;
    jv["address"] = addr.encoded();
    if (get_api_version() == 1) {
        // compatible for version 1: as string value
        jv["confirmed"] = std::to_string(addr_balance.confirmed_balance);
        jv["received"]  = std::to_string(addr_balance.total_received);
        jv["unspent"]   = std::to_string(addr_balance.unspent_balance);
        jv["frozen"]    = std::to_string(addr_balance.frozen_balance);
    }
    else {
        jv["confirmed"] = addr_balance.confirmed_balance;
        jv["received"]  = addr_balance.total_received;
        jv["unspent"]   = addr_balance.unspent_balance;
        jv["frozen"]    = addr_balance.frozen_balance;
    }

    if (get_api_version() <= 2) {
        jv_output["balance"] = jv;
    }
    else {
        jv_output = jv;
    }

    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

