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
#include <metaverse/explorer/extensions/wallet/deposit.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."

#if 0
/************************ deposit *************************/
console_result deposit::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if(!argument_.address.empty() && !blockchain.is_valid_address(argument_.address)) 
        throw address_invalid_exception{"invalid address!"};
    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr) 
        throw address_list_nullptr_exception{"nullptr for address list"};

    if (argument_.deposit != 7 && argument_.deposit != 30 
        && argument_.deposit != 90 && argument_.deposit != 182
        && argument_.deposit != 365)
    {
        throw logic_error{"deposit must be one in [7, 30, 90, 182, 365]."};
    }

    std::list<prikey_amount> palist;

    const char* wallet[4]{"xfetchbalance", nullptr, "-t", "etp"};
    std::stringstream sout;
    std::istringstream sin; 

    // get balance
    for (auto& each : *pvaddr){
        sout.str("");
        wallet[1] = each.get_address().c_str();
        dispatch_command(4, wallet + 0, sin, sout, sout, blockchain);

        pt::ptree pt;
        sin.str(sout.str());
        pt::read_json(sin, pt);
        auto unspent = pt.get<uint64_t>("balance.unspent");
        auto frozen = pt.get<uint64_t>("balance.frozen");
        auto balance = unspent - frozen;
        if (balance){
            palist.push_back({each.get_prv_key(auth_.auth), balance});
        }
    }

    // sort
    palist.sort([](const prikey_amount& first, const prikey_amount& last){
            return first.second < last.second;
            });

    auto random = bc::pseudo_random();
    auto index = random % pvaddr->size();

    // my change
    std::vector<std::string> receiver;
    if(argument_.address.empty())
        receiver.push_back(pvaddr->at(index).get_address() + ":" + std::to_string(argument_.amount));
    else
        receiver.push_back(argument_.address + ":" + std::to_string(argument_.amount));
    
    receiver.push_back(pvaddr->at(index).get_address() + ":" + std::to_string(argument_.fee)); // change

    utxo_helper utxo(std::move(palist), std::move(receiver));
    utxo.set_testnet_rules(blockchain.chain_settings().use_testnet_rules);
    utxo.set_reward(argument_.deposit);

    // send
    send_impl(utxo, blockchain, output, output);

    return console_result::okay;
}
#endif

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

