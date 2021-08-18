/**
 * Copyright (c) 2016-2021 mvs developers
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
#include <metaverse/explorer/extensions/commands/getdid.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

console_result getdid::invoke (Json::Value& jv_output,
                               libbitcoin::server::server_node& node)
{
    Json::Value json_value;

    auto& blockchain = node.chain_impl();

    if (option_.symbol.empty()) {

        auto sh_vec = blockchain.get_registered_dids();

        std::sort(sh_vec->begin(), sh_vec->end());
        // add blockchain dids
        for (auto& elem : *sh_vec) {
            json_value.append(elem.get_symbol());
        }

        if (get_api_version() <= 2) {
            jv_output["dids"] = json_value;
        }
        else {
            jv_output = json_value;
        }
    }
    else {
        auto didSymbol = option_.symbol;
        if (blockchain.is_valid_address(didSymbol)) {
            didSymbol = blockchain.get_did_from_address(didSymbol);
            if (didSymbol.empty()) {
                throw address_not_bound_did_exception{"address is not binded with some did on the blockchain"};
            }
        }

        // check did symbol
        check_did_symbol(didSymbol);

        // check did exists
        if (!blockchain.is_did_exist(didSymbol)) {
            throw did_symbol_notfound_exception{"did symbol does not exist on the blockchain"};
        }

        auto blockchain_dids = blockchain.get_did_history_addresses(didSymbol);
        if (blockchain_dids) {
            Json::Value json_address;
            Json::Value did_data;
            for (auto &did : *blockchain_dids) {
                did_data["address"] = did.get_did().get_address();
                did_data["status"] = did.get_status_string();
                if (get_api_version() >= 3) {
                    did_data["symbol"] = didSymbol;
                }
                json_value.append(did_data);
            }
            
            if (get_api_version() <= 2) {
                jv_output["did"] = didSymbol;
                jv_output["addresses"] = json_value;
            }
            else {
                if(json_value.isNull())
                    json_value.resize(0);

                jv_output = json_value;
            }
        }
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

