/**
 * Copyright (c) 2016-2017 mvs developers 
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
#include <metaverse/explorer/extensions/commands/listmultisig.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

using namespace bc::explorer::config;

console_result listmultisig::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    // parameter account name check
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    Json::Value root, nodes;

    auto multisig_vec = acc->get_multisig_vec();
        
    for(auto& acc_multisig : multisig_vec) {
        Json::Value node, pubkeys;
        node["index"] += acc_multisig.get_index();
        //node["hd_index"] += acc_multisig.get_hd_index();
        node["m"] += acc_multisig.get_m();
        node["n"] += acc_multisig.get_n();
        node["self-publickey"] = acc_multisig.get_pubkey();
        node["description"] = acc_multisig.get_description();
        for(auto& each : acc_multisig.get_cosigner_pubkeys()) {
            pubkeys.append(each);
        }
        node["public-keys"] = pubkeys;
        node["multisig-script"] = acc_multisig.get_multisig_script();
        node["address"] = acc_multisig.get_address();

        nodes.append(node);
    }
    
    root["multisig"] = nodes;    
    output << root.toStyledString();
    
    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

