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
#include <metaverse/explorer/extensions/wallet/getnewaddress.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."

/************************ getnewaddress *************************/

console_result getnewaddress::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
	std::string mnemonic;
	acc->get_mnemonic(auth_.auth, mnemonic);
    if (mnemonic.empty()) { throw std::logic_error("mnemonic empty"); }
    if (!option_.count) { throw std::logic_error("invalid address number parameter"); }
	
    const char* cmds[]{"mnemonic-to-seed", "hd-new", "hd-to-ec", "ec-to-public", "ec-to-address"};
    std::ostringstream sout("");
    std::istringstream sin(mnemonic);

    auto exec_with = [&](int i){
        sin.str(sout.str());
        sout.str("");
        return dispatch_command(1, cmds + i, sin, sout, sout);
    };

	uint32_t idx = 0;
    pt::ptree aroot;
    pt::ptree addresses;

	for ( idx = 0; idx < option_.count; idx++ ) {

	    auto addr = std::make_shared<bc::chain::account_address>();
	    addr->set_name(auth_.name);
		
		sout.str("");
		sin.str(mnemonic);
	    dispatch_command(1, cmds + 0, sin, sout, sout);
	    exec_with(1);

	    auto&& argv_index = std::to_string(acc->get_hd_index());
	    const char* hd_private_gen[3] = {"hd-private", "-i", argv_index.c_str()};
	    sin.str(sout.str());
	    sout.str("");
	    dispatch_command(3, hd_private_gen, sin, sout, sout);

	    exec_with(2);
	    addr->set_prv_key(sout.str(), auth_.auth);
		// not store public key now
	    exec_with(3);
	    //addr->set_pub_key(sout.str());

	    // testnet
	    if (blockchain.chain_settings().use_testnet_rules){
	        const char* cmds_tn[]{"ec-to-address", "-v", "127"};
	        sin.str(sout.str());
	        sout.str("");
	        dispatch_command(3, cmds_tn, sin, sout, sout);
	    // mainnet
	    } else {
	        exec_with(4);
	    }

	    addr->set_address(sout.str());
	    addr->set_status(1); // 1 -- enable address
	    //output<<sout.str();

	    acc->increase_hd_index();
	    addr->set_hd_index(acc->get_hd_index());
	    blockchain.store_account(acc);
	    blockchain.store_account_address(addr);
		// write to output json
        pt::ptree address;
        address.put("", sout.str());
        addresses.push_back(std::make_pair("", address));
	}
	
    aroot.add_child("addresses", addresses);
    pt::write_json(output, aroot);
	
    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

