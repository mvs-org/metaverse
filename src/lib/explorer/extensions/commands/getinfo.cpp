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

#include <metaverse/explorer/json_helper.hpp>
#include <metaverse/explorer/version.hpp>
#include <metaverse/explorer/extensions/commands/getinfo.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/node_method_wrapper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ getinfo *************************/

console_result getinfo::invoke(Json::Value& jv_output,
                               libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();

    administrator_required_checker(node, auth_.name, auth_.auth);

    auto sh_vec = blockchain.get_issued_assets();
    std::set<std::string> symbols;
    for (const auto& elem : *sh_vec) {
        symbols.insert(elem.get_symbol());
    }

    uint64_t height;
    uint64_t rate;
    std::string difficulty;
    bool is_solo_mining;
    node.miner().get_state(height, rate, difficulty, is_solo_mining);

    auto& jv = jv_output;
    if (get_api_version() <= 2) {
        jv["protocol-version"] = node.network_settings().protocol;
        jv["wallet-version"] = MVS_EXPLORER_VERSION;
        jv["database-version"] = MVS_DATABASE_VERSION;
        jv["testnet"] = blockchain.chain_settings().use_testnet_rules;
        jv["peers"] = get_connections_count(node);

        jv["network-assets-count"] = static_cast<uint64_t>(symbols.size());
        jv["wallet-account-count"] = static_cast<uint64_t>(blockchain.get_accounts()->size());

        jv["height"] = height;
        jv["difficulty"] = difficulty;
        jv["is-mining"] = is_solo_mining;
        jv["hash-rate"] = rate;
    }
    else {
        jv["protocol_version"] = node.network_settings().protocol;
        jv["wallet_version"] = MVS_EXPLORER_VERSION;
        jv["database_version"] = MVS_DATABASE_VERSION;
        jv["testnet"] = blockchain.chain_settings().use_testnet_rules;
        jv["peers"] = get_connections_count(node);

        jv["network_assets_count"] = static_cast<uint64_t>(symbols.size());
        jv["wallet_account_count"] = static_cast<uint64_t>(blockchain.get_accounts()->size());

        jv["height"] = height;
        jv["difficulty"] = difficulty;
        jv["is_mining"] = is_solo_mining;
        jv["hash_rate"] = rate;
    }

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

