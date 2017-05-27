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

#include <metaverse/explorer/extensions/commands/private_send.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>
#include <metaverse/explorer/prop_tree.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."

/************************ deposit *************************/
console_result deposit::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if(!argument_.address.empty() && !blockchain.is_valid_address(argument_.address)) 
        throw std::logic_error{"invalid address!"};

    if (argument_.deposit != 7 && argument_.deposit != 30 
		&& argument_.deposit != 90 && argument_.deposit != 182
		&& argument_.deposit != 365)
    {
        throw std::logic_error{"deposit must be one in [7, 30, 90, 182, 365]."};
    }
	
    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr || pvaddr->empty()) 
        throw std::logic_error{"nullptr for address list"};

    auto random = bc::pseudo_random();
    auto index = random % pvaddr->size();

    auto addr = argument_.address;
	if(addr.empty())
		addr = pvaddr->at(index).get_address();
		
	// receiver
	std::vector<receiver_record> receiver{
		{addr, "", argument_.amount, 0, utxo_attach_type::deposit, attachment()} 
	};
	auto deposit_helper = depositing_etp(blockchain, std::move(auth_.name), std::move(auth_.auth), 
			std::move(addr), std::move(receiver), argument_.deposit, argument_.fee);
			
	deposit_helper.exec();

	// json output
	auto tx = deposit_helper.get_transaction();
	pt::write_json(output, prop_tree(tx, true));

    return console_result::okay;
}

/************************ send *************************/

console_result send::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if (!blockchain.is_valid_address(argument_.address))
        throw std::logic_error{std::string("invalid address : ") + argument_.address};
	
	// receiver
	std::vector<receiver_record> receiver{
		{argument_.address, "", argument_.amount, 0, utxo_attach_type::etp, attachment()}  
	};
	auto send_helper = sending_etp(blockchain, std::move(auth_.name), std::move(auth_.auth), 
			"", std::move(receiver), argument_.fee);
	
	send_helper.exec();

	// json output
	auto tx = send_helper.get_transaction();
	pt::write_json(output, prop_tree(tx, true));

	return console_result::okay;
}

/************************ sendmore *************************/

console_result sendmore::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

	// receiver
	receiver_record record;
	std::vector<receiver_record> receiver;
	
	for( auto& each : argument_.receivers){
		colon_delimited2_item<std::string, uint64_t> item(each);
		record.target = item.first();
		// address check
		if (!blockchain.is_valid_address(record.target))
			throw std::logic_error{std::string("invalid address!") + record.target};
		record.symbol = "";
		record.amount = item.second();
		record.asset_amount = 0;
		record.type = utxo_attach_type::etp; // attach not used so not do attah init
		receiver.push_back(record);
	}
	auto send_helper = sending_etp(blockchain, std::move(auth_.name), std::move(auth_.auth), 
			"", std::move(receiver), argument_.fee);
	
	send_helper.exec();

	// json output
	auto tx = send_helper.get_transaction();
	pt::write_json(output, prop_tree(tx, true));

    return console_result::okay;
}

/************************ sendfrom *************************/
console_result sendfrom::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if(!blockchain.is_valid_address(argument_.from)) 
        throw std::logic_error{"invalid from address!"};
    if(!blockchain.is_valid_address(argument_.to)) 
        throw std::logic_error{"invalid to address!"};
    
	// receiver
	std::vector<receiver_record> receiver{
		{argument_.to, "", argument_.amount, 0, utxo_attach_type::etp, attachment()}  
	};
	auto send_helper = sending_etp(blockchain, std::move(auth_.name), std::move(auth_.auth), 
			std::move(argument_.from), std::move(receiver), argument_.fee);
	
	send_helper.exec();

	// json output
	auto tx = send_helper.get_transaction();
	pt::write_json(output, prop_tree(tx, true));

    return console_result::okay;
}

/************************ sendwithmsg *************************/

console_result sendwithmsg::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
	blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

	// receiver
	std::vector<receiver_record> receiver{
		{argument_.address, "", argument_.amount, 0, utxo_attach_type::etp, attachment()},  
		{argument_.address, "", 0, 0, utxo_attach_type::message, attachment(0, 0, blockchain_message(argument_.message))}  
	};
	auto send_helper = sending_etp(blockchain, std::move(auth_.name), std::move(auth_.auth), 
			"", std::move(receiver), argument_.fee);
	
	send_helper.exec();

	// json output
	auto tx = send_helper.get_transaction();
	pt::write_json(output, prop_tree(tx, true));

	return console_result::okay;
}

/************************ sendwithmsgfrom *************************/

console_result sendwithmsgfrom::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
	blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
	if(!blockchain.is_valid_address(argument_.from)) 
		throw std::logic_error{"invalid from address!"};
	if(!blockchain.is_valid_address(argument_.to)) 
		throw std::logic_error{"invalid to address!"};
	
	// receiver
	std::vector<receiver_record> receiver{
		{argument_.to, "", argument_.amount, 0, utxo_attach_type::etp, attachment()},  
		{argument_.to, "", 0, 0, utxo_attach_type::message, attachment(0, 0, blockchain_message(argument_.message))}  
	};
	auto send_helper = sending_etp(blockchain, std::move(auth_.name), std::move(auth_.auth), 
			std::move(argument_.from), std::move(receiver), argument_.fee);
	
	send_helper.exec();

	// json output
	auto tx = send_helper.get_transaction();
	pt::write_json(output, prop_tree(tx, true));

	return console_result::okay;
}

/************************ issue *************************/

console_result issue::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(argument_.symbol);

	if(argument_.fee < 1000000000)
        throw std::logic_error{"issue asset fee less than 1000000000!"};
    if (argument_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw std::logic_error{"asset symbol length must be less than 64."};
    // fail if asset is already in blockchain
    if(blockchain.is_asset_exist(argument_.symbol, false))
        throw std::logic_error{"asset symbol is already exist in blockchain"};
	// local database asset check
	auto sh_asset = blockchain.get_account_asset(auth_.name, argument_.symbol);
	if(sh_asset->empty())
		throw std::logic_error{argument_.symbol + " not found"};
	if(asset_detail::asset_detail_type::created != sh_asset->at(0).detail.get_asset_type())
		throw std::logic_error{argument_.symbol + " has been issued"};

    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr || pvaddr->empty()) 
        throw std::logic_error{"nullptr for address list"};
    
    // get random address    
    auto index = bc::pseudo_random() % pvaddr->size();
    auto addr = pvaddr->at(index).get_address();
	
	// receiver
	std::vector<receiver_record> receiver{
		{addr, argument_.symbol, 0, 0, utxo_attach_type::asset_issue, attachment()}  
	};
	auto issue_helper = issuing_asset(blockchain, std::move(auth_.name), std::move(auth_.auth), 
			"", std::move(argument_.symbol), std::move(receiver), argument_.fee);
	
	issue_helper.exec();

	// json output
	auto tx = issue_helper.get_transaction();
	pt::write_json(output, prop_tree(tx, true));

	// change asset status
	sh_asset->at(0).detail.set_asset_type(asset_detail::asset_detail_type::issued_not_in_blockchain);
	auto detail = std::make_shared<asset_detail>(sh_asset->at(0).detail);
    blockchain.store_account_asset(detail);

    return console_result::okay;
}

/************************ issuefrom *************************/

console_result issuefrom::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(argument_.symbol);

	if(argument_.fee < 1000000000)
        throw std::logic_error{"issue asset fee less than 1000000000!"};
    if (argument_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw std::logic_error{"asset symbol length must be less than 64."};
    if (!blockchain.is_valid_address(argument_.address))
        throw std::logic_error{"invalid address parameter!"};
    // fail if asset is already in blockchain
    if(blockchain.is_asset_exist(argument_.symbol, false))
        throw std::logic_error{"asset symbol is already exist in blockchain"};

	// local database asset check
	auto sh_asset = blockchain.get_account_asset(auth_.name, argument_.symbol);
	if(sh_asset->empty())
		throw std::logic_error{argument_.symbol + " not found"};
	if(asset_detail::asset_detail_type::created != sh_asset->at(0).detail.get_asset_type())
		throw std::logic_error{argument_.symbol + " has been issued"};

	// receiver
	std::vector<receiver_record> receiver{
		{argument_.address, argument_.symbol, 0, 0, utxo_attach_type::asset_issue, attachment()}  
	};
	auto issue_helper = issuing_asset(blockchain, std::move(auth_.name), std::move(auth_.auth), 
			std::move(argument_.address), std::move(argument_.symbol), std::move(receiver), argument_.fee);
	
	issue_helper.exec();
	// json output
	auto tx = issue_helper.get_transaction();
#if 0
	auto issue_helper = issuing_locked_asset(blockchain, std::move(auth_.name), std::move(auth_.auth), 
			std::move(argument_.address), std::move(argument_.symbol), std::move(receiver), argument_.fee, argument_.lockedtime);
	
	issue_helper.exec();
	// json output
	auto tx = issue_helper.get_transaction();
#endif
	// change asset status
	sh_asset->at(0).detail.set_asset_type(asset_detail::asset_detail_type::issued_not_in_blockchain);
	auto detail = std::make_shared<asset_detail>(sh_asset->at(0).detail);
    blockchain.store_account_asset(detail);

	pt::write_json(output, prop_tree(tx, true));

    return console_result::okay;
}

/************************ issuemore *************************/

console_result issuemore::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}


/************************ issuemorefrom *************************/

console_result issuemorefrom::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    output << IN_DEVELOPING;
    return console_result::okay;
}

/************************ sendasset *************************/

console_result sendasset::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
	blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
	blockchain.uppercase_symbol(argument_.symbol);
	
	if (argument_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
		throw std::logic_error{"asset symbol length must be less than 64."};
	if (!blockchain.is_valid_address(argument_.address))
		throw std::logic_error{"invalid to address parameter!"};
	if (!argument_.amount)
		throw std::logic_error{"invalid asset amount parameter!"};

	// receiver
	std::vector<receiver_record> receiver{
		{argument_.address, argument_.symbol, 0, argument_.amount, utxo_attach_type::asset_transfer, attachment()}  
	};
	auto send_helper = sending_asset(blockchain, std::move(auth_.name), std::move(auth_.auth), 
			"", std::move(argument_.symbol), std::move(receiver), argument_.fee);
#if 0
	auto send_helper = sending_locked_asset(blockchain, std::move(auth_.name), std::move(auth_.auth), 
			"", std::move(argument_.symbol), std::move(receiver), argument_.fee, argument_.lockedtime);
#endif
	
	send_helper.exec();

	// json output
	auto tx = send_helper.get_transaction();
	pt::write_json(output, prop_tree(tx, true));

	return console_result::okay;
}

/************************ sendassetfrom *************************/

console_result sendassetfrom::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(argument_.symbol);
    
    if (argument_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw std::logic_error{"asset symbol length must be less than 64."};
    
    if (!blockchain.is_valid_address(argument_.from))
        throw std::logic_error{"invalid from address parameter!"};
    if (!blockchain.is_valid_address(argument_.to))
        throw std::logic_error{"invalid to address parameter!"};
    if (!argument_.amount)
        throw std::logic_error{"invalid asset amount parameter!"};

	// receiver
	std::vector<receiver_record> receiver{
		{argument_.to, argument_.symbol, 0, argument_.amount, utxo_attach_type::asset_transfer, attachment()}  
	};
	auto send_helper = sending_asset(blockchain, std::move(auth_.name), std::move(auth_.auth), 
			std::move(argument_.from), std::move(argument_.symbol), std::move(receiver), argument_.fee);
	
	send_helper.exec();

	// json output
	auto tx = send_helper.get_transaction();
	pt::write_json(output, prop_tree(tx, true));

    return console_result::okay;
}

} //commands
} // explorer
} // libbitcoin
