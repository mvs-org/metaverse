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
#include <metaverse/explorer/extensions/wallet/sendassetfrom.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."
#if 0
/************************ sendassetfrom *************************/

console_result sendassetfrom::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)

{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(argument_.symbol);
    
    if (argument_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw asset_symbol_length_exception{"asset symbol length must be less than 64."};
    
    if (!blockchain.is_valid_address(argument_.from))
        throw fromaddress_invalid_exception{"invalid from address parameter!"};
    if (!blockchain.is_valid_address(argument_.to))
        throw toaddress_invalid_exception{"invalid to address parameter!"};
    if (!argument_.amount)
        throw asset_amount_exception{"invalid asset amount parameter!"};

    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr) 
        throw address_list_nullptr_exception{"nullptr for address list"};
    
    auto kind = business_kind::asset_issue;
    std::list<prikey_etp_amount> asset_ls;
    //std::shared_ptr<std::vector<business_history>> 
    auto sh_vec = blockchain.get_address_business_history(argument_.from, argument_.symbol, kind, business_status::unspent);
	if(sh_vec->size()){ // asset issue utxo exist
		for (auto& each : *pvaddr){
			if ( 0 != argument_.from.compare(each.get_address()) )
				continue;
			const auto sum = [&](const business_history& bh)
			{
				auto asset_info = boost::get<asset_detail>(bh.data.get_data());
				asset_ls.push_back({each.get_prv_key(auth_.auth), bh.value, asset_info.get_maximum_supply(), bh.output});
			};
			std::for_each(sh_vec->begin(), sh_vec->end(), sum);
			break;
		}
	} else { // not found issue asset then search transfer asset 
        kind = business_kind::asset_transfer;
        sh_vec = blockchain.get_address_business_history(argument_.from, argument_.symbol, kind, business_status::unspent);
        for (auto& each : *pvaddr){
            if ( 0 != argument_.from.compare(each.get_address()) )
                continue;
            const auto sum = [&](const business_history& bh)
            {
                auto transfer_info = boost::get<asset_transfer>(bh.data.get_data());
                asset_ls.push_back({each.get_prv_key(auth_.auth), bh.value, transfer_info.get_quantity(), bh.output});
            };
            std::for_each(sh_vec->begin(), sh_vec->end(), sum);
        }
    }    
#ifdef MVS_DEBUG
    /* debug code begin */    
    const auto action = [&](business_history& elem)
    {
        log::info("sendassetfrom") <<elem.to_string();
    };
    std::for_each(sh_vec->begin(), sh_vec->end(), action);
    /* debug code end */
#endif
    
    if(!asset_ls.size()) 
        throw tx_source_exception{"no asset business for from address!"};
	
	// etp check incase from address etp < fee
    uint64_t total_balance = 0;
    for (auto& each : asset_ls){
        total_balance += each.value;
    }
	if(total_balance < argument_.fee) {
		total_balance = 0;
		for (auto& each : *pvaddr){
			if ( 0 != argument_.from.compare(each.get_address()) )
				continue;
			auto etp_ls = blockchain.get_address_business_history(argument_.from, "", business_kind::etp, business_status::unspent);
			for( auto& bh : *etp_ls){
				total_balance += bh.value; 
				asset_ls.push_back({each.get_prv_key(auth_.auth), bh.value, 0, bh.output});
				if(total_balance >= argument_.fee)
					break;
			}
			break;
		}
	}

	// send
    std::string type("asset-transfer");
    auto testnet_rules = blockchain.chain_settings().use_testnet_rules;
    utxo_attach_sendfrom_helper utxo(std::move(auth_.name), std::move(auth_.auth), std::move(type), std::move(asset_ls), 
        argument_.fee, std::move(argument_.symbol), argument_.amount, std::move(argument_.to), testnet_rules);

    send_impl(utxo, blockchain, output, output);

    return console_result::okay;
}
#endif

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

