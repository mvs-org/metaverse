/**
 * Copyright (c) 2016-2020 mvs developers
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
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/burn.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

console_result burn::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    std::string blackhole_did = chain::did_detail::get_blackhole_did_symbol();

    if (option_.is_mit) {
        const char* cmds[] {
            "transfermit", auth_.name.c_str(), auth_.auth.c_str(),
            blackhole_did.c_str(), argument_.symbol.c_str()
        };

        return dispatch_command(5, cmds, jv_output, node, get_api_version());
    }
    else if (!option_.cert_type.empty()) {
        const char* cmds[] {
            "transfercert", auth_.name.c_str(), auth_.auth.c_str(),
            blackhole_did.c_str(), argument_.symbol.c_str(), option_.cert_type.c_str()
        };

        return dispatch_command(6, cmds, jv_output, node, get_api_version());
    }
    else {
        if (argument_.amount <= 0) {
            throw argument_legality_exception{"invalid amount parameter!"};
        }

        auto&& amount = std::to_string(argument_.amount);
        const char* cmds[] {
            "sendasset", auth_.name.c_str(), auth_.auth.c_str(),
            blackhole_did.c_str(), argument_.symbol.c_str(), amount.c_str()
        };

        return dispatch_command(6, cmds, jv_output, node, get_api_version());
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

