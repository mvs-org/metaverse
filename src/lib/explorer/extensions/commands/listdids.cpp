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
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/listdids.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ listdids *************************/

console_result listdids::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    Json::Value dids;
    auto& blockchain = node.chain_impl();

    std::shared_ptr<did_detail::list> sh_vec;
    if (auth_.name.empty()) {
        // no account -- list all dids in blockchain
        sh_vec = blockchain.get_registered_dids();
    }
    else {
        // list dids owned by the account
        blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
        sh_vec = blockchain.get_account_dids(auth_.name);
    }

    std::sort(sh_vec->begin(), sh_vec->end());

    // add blockchain dids
    for (auto& elem: *sh_vec) {
        Json::Value did_data;
        did_data["symbol"] = elem.get_symbol();
        did_data["address"] = elem.get_address();
        did_data["status"] = "registered";
        dids.append(did_data);
    }

    if (get_api_version() == 1 && dids.isNull()) { //compatible for v1
        jv_output["dids"] = "";
    }
    else if (get_api_version() <= 2) {
        jv_output["dids"] = dids;
    }
    else {
        if(dids.isNull())
            dids.resize(0);  

        jv_output = dids;
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin
