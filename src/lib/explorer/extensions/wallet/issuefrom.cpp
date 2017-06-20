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
#include <metaverse/explorer/extensions/wallet/issuefrom.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."

/************************ issuefrom *************************/
#if 0
console_result issuefrom::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
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
    auto pvaddr = blockchain.get_account_addresses(auth_.name);
    if(!pvaddr) 
        throw std::logic_error{"nullptr for address list"};
    
    auto sh_vec = blockchain.get_account_asset(auth_.name, argument_.symbol);
    log::debug("issue") << "asset size = " << sh_vec->size();
    if(!sh_vec->size())
        throw std::logic_error{"no such asset"};

#ifdef MVS_DEBUG
    /* debug code begin */    
    const auto action = [&](business_address_asset& elem)
    {
        log::debug("issuefrom") <<elem.to_string();
    };
    std::for_each(sh_vec->begin(), sh_vec->end(), action);
    /* debug code end */    
#endif

    std::list<prikey_amount> palist;

    const char* wallet[4]{"xfetchbalance", nullptr, "-t", "etp"}; // only spent pure etp utxo
    std::ostringstream sout;
    std::istringstream sin; 

    // get balance
    for (auto& each : *pvaddr){
        if( 0 == each.get_address().compare(argument_.address) ) {
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
            }else{
                throw std::logic_error{"no enough balance"};
            }
            break;
    }
    }
    // address check
    if(palist.empty())
        throw std::logic_error{"no such address"};
    
#ifdef MVS_DEBUG
    /* debug code begin */    
    const auto gaction = [&](prikey_amount& elem)
    {
        log::debug("issuefrom") <<"palist="<<elem.first<< " "<<elem.second;
    };
    std::for_each(palist.begin(), palist.end(), gaction);
    /* debug code end */    
#endif
    
    // send
    std::string type("asset-issue");
    auto testnet_rules = blockchain.chain_settings().use_testnet_rules;

    utxo_attach_issuefrom_helper utxo(std::move(auth_.name), std::move(auth_.auth), std::move(type), std::move(palist), 
        std::move(argument_.address), argument_.fee, std::move(argument_.symbol), sh_vec->begin()->quantity, testnet_rules);

    send_impl(utxo, blockchain, output, output);

    return console_result::okay;
}
#endif



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

