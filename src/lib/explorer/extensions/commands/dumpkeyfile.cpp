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


#include <metaverse/explorer/extensions/commands/dumpkeyfile.hpp>
#include <metaverse/explorer/extensions/account_info.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
namespace fs = boost::filesystem;
using namespace bc::explorer::config;
/************************ exportaccountasfile *************************/

console_result dumpkeyfile::invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    std::string mnemonic;
    acc->get_mnemonic(auth_.auth, mnemonic);
    std::vector<std::string> results;
    boost::split(results, mnemonic, boost::is_any_of(" "));

    if (*results.rbegin() != argument_.last_word){
        throw argument_legality_exception{"last word not matching."};
    }
    
    fs::file_status status = fs::status(argument_.dst); 
    if(fs::is_directory(status)) // not process filesystem exception here
        argument_.dst /= auth_.name;
    
    fs::file_status status2 = fs::status(argument_.dst.parent_path()); 
    if(!fs::exists(status2))
        throw argument_legality_exception{argument_.dst.parent_path().string() + std::string(" directory not exist.")};
        
    acc->set_mnemonic(mnemonic); // reset mnemonic to plain text

    // account address info
    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr) throw address_list_nullptr_exception{"nullptr for address list"};

    std::string prv_key;
    for (auto& each : *pvaddr){
        prv_key = each.get_prv_key(auth_.auth);
        each.set_prv_key(prv_key); // reset private key to plain text
    }

    // account asset info
    auto sh_asset_vec = std::make_shared<std::vector<asset_detail>>();
    auto sh_unissued = blockchain.get_account_unissued_assets(auth_.name);        
    for (auto& elem: *sh_unissued) {
        sh_asset_vec->push_back(elem.detail);         
    }
    account_info all_info(blockchain, auth_.auth, *acc, *pvaddr, *sh_asset_vec);

    // store encrypted data to file
    bc::ofstream file_output(argument_.dst.string(), std::ofstream::out);
    file_output << all_info << std::flush;
    file_output.close();

    auto& root = jv_output;
    root["result"] = argument_.dst.string();

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

