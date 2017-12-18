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
#include <metaverse/explorer/extensions/commands/importkeyfile.hpp>
#include <metaverse/explorer/extensions/account_info.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
namespace fs = boost::filesystem;
using namespace bc::explorer::config;

/************************ importkeyfile *************************/

console_result importkeyfile::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{    
    auto& blockchain = node.chain_impl();
    
    fs::file_status status = fs::status(option_.file); 
    if(!fs::exists(status)) // not process filesystem exception here
        throw argument_legality_exception{option_.file.string() + std::string(" not exist.")};
    if(fs::is_directory(status)) // directory not allowed
        throw argument_legality_exception{option_.file.string() + std::string(" is directory not a file.")};
    if(!fs::is_regular_file(status)) 
        throw argument_legality_exception{option_.file.string() + std::string(" not a regular file.")};

    // decrypt account info file first
    account_info all_info(blockchain, option_.depasswd);
    std::ifstream file_input(option_.file.string(), std::ofstream::in);
    file_input >> all_info;
    file_input.close();

    // name check
    auto acc = all_info.get_account();
    auto name = acc.get_name();
    auto mnemonic = acc.get_mnemonic();
    if (blockchain.is_account_exist(name))
        throw account_existed_exception{name + std::string(" account is already exist")};

    // store account info to db
    all_info.store(name, option_.depasswd);
        
    Json::Value root;
    root["name"] = name;
    //root["mnemonic-key"] = mnemonic;
    root["address-count"] += acc.get_hd_index();
    root["unissued-asset-count"] += all_info.get_account_asset().size();
    output << root.toStyledString();

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

