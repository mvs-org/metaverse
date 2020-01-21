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

#include <metaverse/explorer/extensions/commands/registerwitness.hpp>
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/json_helper.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/consensus/witness.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ registerwitness *************************/

console_result registerwitness::invoke(Json::Value& jv_output,
                                     libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    auto didSymbol = argument_.did;

    // check did symbol
    check_did_symbol(didSymbol);

    // check did exists
    if (!blockchain.is_did_exist(didSymbol)) {
        throw did_symbol_notfound_exception{"did " + didSymbol + " does not exist on the blockchain"};
    }

    // check witness_registry_did exists
    auto witness_registry_did = consensus::witness::witness_registry_did;
    if (!blockchain.is_did_exist(witness_registry_did)) {
        throw did_symbol_notfound_exception{"did " + witness_registry_did + " does not exist on the blockchain"};
    }

    auto amount = std::to_string(consensus::witness::witness_register_fee);
    const char* cmds[] {
        "sendfrom", auth_.name.c_str(), auth_.auth.c_str(),
        didSymbol.c_str(), witness_registry_did.c_str(), amount.c_str()
    };

    return dispatch_command(6, cmds, jv_output, node, get_api_version());
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

