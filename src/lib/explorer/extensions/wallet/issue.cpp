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
#include <metaverse/explorer/extensions/wallet/issue.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."

/************************ issue *************************/
#if 0
console_result issue::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr || pvaddr->empty()) 
        throw address_list_nullptr_exception{"nullptr for address list"};
    
    std::vector<prikey_amount> pavec;

    const char* fetch_wallet[2]{"xfetchbalance", nullptr};
    std::ostringstream sout;
    std::istringstream sin; 

    // get balance
    for (auto& each : *pvaddr){
        sout.str("");
        fetch_wallet[1] = each.get_address().c_str();
        dispatch_command(2, fetch_wallet + 0, sin, sout, sout, blockchain);

        pt::ptree pt;
        sin.str(sout.str());
        pt::read_json(sin, pt);
		auto unspent = pt.get<uint64_t>("balance.unspent");
		auto frozen = pt.get<uint64_t>("balance.frozen");
        auto balance = unspent - frozen;
        if (balance){
            pavec.push_back({each.get_address(), balance});
        }
    }
    if(!pavec.size())
        throw lack_account_etp_exception{"not enough etp in your account!"};

    // get random address    
    auto index = bc::pseudo_random() % pavec.size();
    auto addr = pavec.at(index).first;
    
    // make issuefrom command 
    // eg : issuefrom m m t9Vns3EmKtreq68GEMiq5njs4egGc623hm CAR -f fee
    const char* wallet[256]{0x00};
    int i = 0;
    wallet[i++] = "issuefrom";
    wallet[i++] = auth_.name.c_str();
    wallet[i++] = auth_.auth.c_str();
    wallet[i++] = addr.c_str();
    wallet[i++] = argument_.symbol.c_str();
    wallet[i++] = "-f";
    wallet[i++] = std::to_string(argument_.fee).c_str();

    // exec command
    sin.str("");
    sout.str("");
    
	if (dispatch_command(i, wallet, sin, sout, sout, blockchain) != console_result::okay) {
		throw asset_issue_exception(sout.str());
	}
	std::pair<uint32_t, std::string> ex_pair;
	std::stringstream ex_stream;
	ex_stream.str(sout.str());
	if (capture_excode(ex_stream, ex_pair) == console_result::okay) {
		throw explorer_exception(ex_pair.first, ex_pair.second);
	}
    
    output<<sout.str();
    return console_result::okay;
}
#endif

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

