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
#include <metaverse/explorer/extensions/wallet/getbalance.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."

/************************ getbalance *************************/

console_result getbalance::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    pt::ptree aroot;

    auto vaddr = blockchain.get_account_addresses(auth_.name);
    if(!vaddr) throw std::logic_error{"nullptr for address list"};

    const char* wallet[2]{"xfetchbalance", nullptr};
    std::ostringstream sout;
    std::istringstream sin; 
    uint64_t total_confirmed = 0;
    uint64_t total_received = 0;
    uint64_t total_unspent = 0;
	uint64_t total_frozen = 0;

    for (auto& i: *vaddr){
        sout.str("");
        wallet[1] = i.get_address().c_str();
        dispatch_command(2, wallet + 0, sin, sout, sout, blockchain);

        pt::ptree utxo;
        sin.str(sout.str());
        pt::read_json(sin, utxo);
        total_confirmed += utxo.get<uint64_t>("balance.confirmed");
        total_received += utxo.get<uint64_t>("balance.received");
        total_unspent += utxo.get<uint64_t>("balance.unspent");
		total_frozen += utxo.get<uint64_t>("balance.frozen");
    }
    
    aroot.put("total-confirmed", total_confirmed);
    aroot.put("total-received", total_received);
    aroot.put("total-unspent", total_unspent);
	aroot.put("total-frozen", total_frozen);
    pt::write_json(output, aroot);

    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

