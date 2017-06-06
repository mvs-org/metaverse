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
#include <metaverse/explorer/extensions/wallet/xfetchbalance.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."

/************************ xfetchbalance *************************/
console_result xfetchbalance::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
	using namespace bc::client;

	// Bound parameters.
    if (!blockchain.is_valid_address(argument_.address))
        throw std::logic_error{"invalid address parameter!"};
	auto addr_str = argument_.address;
	auto address = payment_address(argument_.address);
	auto type = option_.type;
	const auto connection = get_connection(*this);

	obelisk_client client(connection);

	if (!client.connect(connection))
	{
		display_connection_failure(cerr, connection.server);
		return console_result::failure;
	}

	auto on_done = [&addr_str, &type, &output, &blockchain](const history::list& rows)
	{
		pt::ptree tree;
		pt::ptree balance;
		uint64_t total_received = 0;
		uint64_t confirmed_balance = 0;
		uint64_t unspent_balance = 0;
		uint64_t frozen_balance = 0;
		
		chain::transaction tx_temp;
		uint64_t tx_height; // unused
		uint64_t height = 0;
		blockchain.get_last_height(height);

		for (auto& row: rows)
		{
			total_received += row.value;
		
			// spend unconfirmed (or no spend attempted)
			if (row.spend.hash == null_hash) {
				blockchain.get_transaction(row.output.hash, tx_temp, tx_height); // todo -- return value check
				auto output = tx_temp.outputs.at(row.output.index);

				// deposit utxo in transaction pool
				if ((output.script.pattern() == bc::chain::script_pattern::pay_key_hash_with_lock_height)
							&& !row.output_height) { 
					frozen_balance += row.value;
			    }

				// deposit utxo in block
				if(chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations)
					&& row.output_height) { 
					uint64_t lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(output.script.operations);
					if((row.output_height + lock_height) > height) { // utxo already in block but deposit not expire
						frozen_balance += row.value;
					}
				}
				
				// coin base etp maturity etp check
				if(tx_temp.is_coinbase()
					&& !(output.script.pattern() == bc::chain::script_pattern::pay_key_hash_with_lock_height)) { // incase readd deposit
					// add not coinbase_maturity etp into frozen
					if((!row.output_height ||
								(row.output_height && (height - row.output_height) < coinbase_maturity))) {
						frozen_balance += row.value;
					}
				}
				
				if((type == "all") 
					|| ((type == "etp") && output.is_etp()))
					unspent_balance += row.value;
			}
		
			if (row.output_height != 0 &&
				(row.spend.hash == null_hash || row.spend_height == 0))
				confirmed_balance += row.value;
		}
		
		balance.put("address", addr_str);
		balance.put("confirmed", confirmed_balance);
		balance.put("received", total_received);
		balance.put("unspent", unspent_balance);
		balance.put("frozen", frozen_balance);
		
		tree.add_child("balance", balance);
		pt::write_json(output, tree);
	};

	auto on_error = [&output](const code& error)
	{
		if(error) {
			output<<error.message();
		}
	};

	// The v3 client API works with and normalizes either server API.
	//// client.address_fetch_history(on_error, on_done, address);
	client.address_fetch_history2(on_error, on_done, address);
	client.wait();

    //payment_address address(argument_.address);

	return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

