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


#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/listaddresses.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ listaddresses *************************/

console_result listaddresses::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    const auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    auto& aroot = jv_output;
    Json::Value addresses;

    if (option_.b_script) {
        for (auto sc : acc->get_script_vec()) {
            Json::Value script;
            script["address"] = sc.get_address();
            script["script"] = encode_base16(sc.get_script());
            script["description"] = sc.get_description();

            addresses.append(script);
        }
    } else {
        auto vaddr = blockchain.get_account_addresses(auth_.name);
        if(!vaddr) throw address_list_nullptr_exception{"nullptr for address list"};

        for (auto& it: *vaddr){
            addresses.append(it.get_address());
        }
    }

    

    if (get_api_version() == 1 && addresses.isNull()) { // compatible for v1
        aroot["addresses"] = "";
    }
    else if (get_api_version() <= 2) {
        aroot["addresses"] = addresses;
    }
    else {
        if(addresses.isNull())
            addresses.resize(0);  

        aroot = addresses;
    }

    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

