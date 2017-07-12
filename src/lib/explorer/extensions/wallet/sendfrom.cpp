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

#include <boost/property_tree/ptree.hpp>      
#include <boost/property_tree/json_parser.hpp>

#include <metaverse/bitcoin.hpp>
#include <metaverse/client.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/callback_state.hpp>
#include <metaverse/explorer/display.hpp>
#include <metaverse/explorer/prop_tree.hpp>
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/wallet/sendfrom.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."

/************************ sendfrom *************************/

#if 0

console_result sendfrom::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if(!blockchain.is_valid_address(argument_.from)) 
        throw fromaddress_invalid_exception{"invalid from address!"};
    if(!blockchain.is_valid_address(argument_.to)) 
        throw toaddress_invalid_exception{"invalid to address!"};
    
    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr) 
        throw address_list_nullptr_exception{"nullptr for address list"};

    std::list<prikey_amount> palist;

    const char* wallet[4]{"xfetchbalance", nullptr, "-t", "etp"};
    std::ostringstream sout;
    std::istringstream sin; 

    // get balance
    for (auto& each : *pvaddr){
        if(each.get_address().compare(argument_.from) != 0)
            continue;
        
        sout.str("");
        wallet[1] = each.get_address().c_str();
        dispatch_command(4, wallet + 0, sin, sout, sout, blockchain);

        pt::ptree pt;
        sin.str(sout.str());
        pt::read_json(sin, pt);
        auto unspent = pt.get<uint64_t>("balance.unspent");
        auto frozen = pt.get<uint64_t>("balance.frozen");
        auto balance = unspent - frozen;
        if (balance && (balance >= argument_.fee)){
            palist.push_back({each.get_prv_key(auth_.auth), balance});
        }
    }
    if(palist.empty())
        throw tx_source_exception{"not enough etp in from address or you are't own from address!"};
    
    // my change
    std::vector<std::string> receiver{
        {argument_.to + ":" + std::to_string(argument_.amount)},
        {argument_.from + ":" + std::to_string(argument_.fee)}
    };

    utxo_helper utxo(std::move(palist), std::move(receiver));
    utxo.set_testnet_rules(blockchain.chain_settings().use_testnet_rules);

    // send
    send_impl(utxo, blockchain, output, output);

    return console_result::okay;
}
#endif

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

