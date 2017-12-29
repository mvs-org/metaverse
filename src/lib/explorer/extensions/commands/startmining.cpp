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


#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/startmining.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ startmining *************************/

console_result startmining::invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node)
{
    
    // get new address 
    const char* cmds2[]{"getnewaddress", auth_.name.c_str(), auth_.auth.c_str()};

    auto& blockchain = node.chain_impl();
    auto& miner = node.miner();
    Json::Value jv_temp;

    if (dispatch_command(3, cmds2, jv_temp, node, 2) != console_result::okay) {
        throw address_generate_exception(jv_temp.asString());
    }

    auto&& str_addr = jv_temp["addresses"][0].asString();
     
    bc::wallet::payment_address addr(str_addr);

    // start
    if (miner.start(addr)){
        jv_output = "solo mining started at " + str_addr;
    } else {
        throw unknown_error_exception{"solo mining startup got error"};
    }


    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

