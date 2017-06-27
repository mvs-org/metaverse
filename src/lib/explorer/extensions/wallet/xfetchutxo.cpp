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
#include <metaverse/explorer/extensions/wallet/xfetchutxo.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."

/************************ xfetchutxo *************************/
console_result xfetchutxo::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
	using namespace bc::client;

	// Bound parameters.
	auto& blockchain = node.chain_impl();
	if (!blockchain.is_valid_address(argument_.address))
		throw std::logic_error{"invalid address parameter!"};
	//auto addr_str = argument_.address;
	auto amount = argument_.amount;
	auto address = payment_address(argument_.address);
	auto type = option_.type;
	const auto connection = get_connection(*this);

	obelisk_client client(connection);

	if (!client.connect(connection))
	{
		display_connection_failure(cerr, connection.server);
		return console_result::failure;
	}

	auto on_done = [&amount, &type, &output, &blockchain](const history::list& rows)
	{
		uint64_t height = 0;
		blockchain.get_last_height(height);
		chain::output_info::list unspent;
		chain::transaction tx_temp;
		uint64_t tx_height;
		
		for (auto& row: rows)
		{		
			// spend unconfirmed (or no spend attempted)
			if (row.spend.hash == null_hash) {
				// fetch utxo script to check deposit utxo
				blockchain.get_transaction(row.output.hash, tx_temp, tx_height); // todo -- return value check
				auto output = tx_temp.outputs.at(row.output.index);
				bool is_deposit_utxo = false;

				// deposit utxo in transaction pool
				if ((output.script.pattern() == bc::chain::script_pattern::pay_key_hash_with_lock_height)
							&& !row.output_height) { 
					is_deposit_utxo = true;
			    }

				// deposit utxo in block
				if(chain::operation::is_pay_key_hash_with_lock_height_pattern(output.script.operations)
					&& row.output_height) { 
					uint64_t lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(output.script.operations);
					if((row.output_height + lock_height) > height) { // utxo already in block but deposit not expire
						is_deposit_utxo = true;
					}
				}
				
				// coin base etp maturity etp check
				if(tx_temp.is_coinbase()
					&& !(output.script.pattern() == bc::chain::script_pattern::pay_key_hash_with_lock_height)) { // incase readd deposit
					// add not coinbase_maturity etp into frozen
					if((!row.output_height ||
								(row.output_height && (height - row.output_height) < coinbase_maturity))) {
						is_deposit_utxo = true; // not deposit utxo but can not spent now
					}
				}
				
				if(is_deposit_utxo)
					continue;
				
				if((type == "all") 
					|| ((type == "etp") && output.is_etp()))
					unspent.push_back({row.output, row.value});
			}
		
		}
		
		chain::points_info selected_utxos;
		wallet::select_outputs::select(selected_utxos, unspent, amount);
			
		pt::ptree tree = config::prop_tree(selected_utxos, true); // json format
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

	return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin


