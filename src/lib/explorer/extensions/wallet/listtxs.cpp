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
#include <metaverse/explorer/extensions/wallet/listtxs.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."

/************************ listtxs *************************/

console_result listtxs::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    using namespace libbitcoin::config; // for hash256
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if (!argument_.address.empty() && !blockchain.is_valid_address(argument_.address))
        throw address_invalid_exception{"invalid address parameter!"};
    blockchain.uppercase_symbol(argument_.symbol);

    pt::ptree aroot;
    pt::ptree balances;
	const uint64_t limit = 100;
	uint64_t height = 0;
	
	struct tx_hash_height {
		std::string  hash_;
		uint64_t height_;
	};
	
	auto sort_by_height = [](const tx_hash_height &lhs, const tx_hash_height &rhs)->bool { return lhs.height_ < rhs.height_; };
	
    auto sh_tx_hash = std::make_shared<std::vector<tx_hash_height>>();
    auto sh_txs = std::make_shared<std::vector<tx_block_info>>();
	blockchain.get_last_height(height);
	// 1. no address -- list all account tx
    if(argument_.address.empty()) { 
        auto pvaddr = blockchain.get_account_addresses(auth_.name);
        if(!pvaddr) 
            throw address_list_nullptr_exception{"nullptr for address list"};
        
        for (auto& elem: *pvaddr) {
            auto sh_vec = blockchain.get_address_business_record(elem.get_address(), height, limit);
            // scan all kinds of business
            for (auto each : *sh_vec){
				auto pos = std::find_if(sh_tx_hash->begin(), sh_tx_hash->end(), [&](const tx_hash_height& elem){
						return (elem.hash_ == hash256(each.point.hash).to_string()) && (elem.height_ == each.height);
						});
				
				if (pos == sh_tx_hash->end()){ // new item
					sh_tx_hash->push_back({hash256(each.point.hash).to_string(), each.height});
                    tx_block_info tx;
                    tx.height = each.height;
                    tx.timestamp = each.data.get_timestamp();
                    tx.hash = hash256(each.point.hash).to_string();
                    sh_txs->push_back(tx);
                }
            }
        }
    } else { // address exist in command
        // timestamp parameter check
        auto has_colon = false;
        for (auto& i : argument_.address){
            if (i==':') {
                has_colon = true;
                break;
            }
        }
        // 2. has timestamp, list all account tx between star:end
        if (has_colon) {
            const auto tokens = split(argument_.address, BX_TX_POINT_DELIMITER);
            if (tokens.size() != 2)
            {
                throw format_timestamp_exception{"timestamp is invalid format(eg : 123:456)!"};
            }
            uint32_t start, end;
            deserialize(start, tokens[0], true);
            deserialize(end, tokens[1], true);

            auto pvaddr = blockchain.get_account_addresses(auth_.name);
            if(!pvaddr) 
                throw address_list_nullptr_exception{"nullptr for address list"};

            for (auto& elem: *pvaddr) {
                auto sh_vec = blockchain.get_address_business_record(elem.get_address());
                // scan all kinds of business
                for (auto each : *sh_vec){
                    if((start <= each.data.get_timestamp()) && (each.data.get_timestamp() < end)) {
						auto pos = std::find_if(sh_tx_hash->begin(), sh_tx_hash->end(), [&](const tx_hash_height& elem){
								return (elem.hash_ == hash256(each.point.hash).to_string()) && (elem.height_ == each.height);
								});
						
						if (pos == sh_tx_hash->end()){ // new item
							sh_tx_hash->push_back({hash256(each.point.hash).to_string(), each.height});
							tx_block_info tx;
							tx.height = each.height;
							tx.timestamp = each.data.get_timestamp();
							tx.hash = hash256(each.point.hash).to_string();
							sh_txs->push_back(tx);
						}
                    }
                }
            }
        // 3. list all tx of the address    
        } else {
        
        	log::trace("listtxs") << option_.height.first();
        	log::trace("listtxs") << option_.height.second();
			
        	if(option_.height.first() > option_.height.second()) {
				throw block_height_exception{"invalid height option!"};
			}
            auto sh_vec = blockchain.get_address_business_record(argument_.address);
            // scan all kinds of business
            for (auto each : *sh_vec){
				if((option_.height.first()==0) && (option_.height.second()==0)){ // only address, no height option
					auto pos = std::find_if(sh_tx_hash->begin(), sh_tx_hash->end(), [&](const tx_hash_height& elem){
							return (elem.hash_ == hash256(each.point.hash).to_string()) && (elem.height_ == each.height);
							});
					
					if (pos == sh_tx_hash->end()){ // new item
						sh_tx_hash->push_back({hash256(each.point.hash).to_string(), each.height});
						tx_block_info tx;
						tx.height = each.height;
						tx.timestamp = each.data.get_timestamp();
						tx.hash = hash256(each.point.hash).to_string();
						sh_txs->push_back(tx);
					}
				} else {  // address with height option
					if((option_.height.first() <= each.height) && (each.height < option_.height.second())) {
						auto pos = std::find_if(sh_tx_hash->begin(), sh_tx_hash->end(), [&](const tx_hash_height& elem){
								return (elem.hash_ == hash256(each.point.hash).to_string()) && (elem.height_ == each.height);
								});
						
						if (pos == sh_tx_hash->end()){ // new item
							sh_tx_hash->push_back({hash256(each.point.hash).to_string(), each.height});
							tx_block_info tx;
							tx.height = each.height;
							tx.timestamp = each.data.get_timestamp();
							tx.hash = hash256(each.point.hash).to_string();
							sh_txs->push_back(tx);
						}
					}
				}
            }
        }
    }
	// sort by height
	std::sort (sh_tx_hash->begin(), sh_tx_hash->end(), sort_by_height);

    // fetch tx according its hash
    std::vector<std::string> vec_ip_addr; // input addr
    std::vector<std::string> vec_op_addr; // output addr
	chain::transaction tx;
	uint64_t tx_height;
	hash_digest trans_hash;
	
    for (auto& each: *sh_txs){
		decode_hash(trans_hash, each.hash);
		if(!blockchain.get_transaction(trans_hash, tx, tx_height))
			continue;

		// filter asset symbol
		if(!argument_.symbol.empty()) {
			auto asset_found = false;
			for(auto& op : tx.outputs) {
	            if((op.get_asset_symbol() == argument_.symbol)) {
					asset_found = true;
					break;
	            }
	        }
			if(!asset_found) // not found asset
				continue;
		}
		
		pt::ptree tx_item;
        tx_item.put("hash", each.hash);
        tx_item.put("height", each.height);
        tx_item.put("timestamp", each.timestamp);
        tx_item.put("direction", "send");

        // set inputs content
        pt::ptree input_addrs;
        for(auto& input : tx.inputs) {
            pt::ptree input_addr;
            std::string addr="";
			
			auto script_address = payment_address::extract(input.script);
			if (script_address)
				addr = script_address.encoded();

            input_addr.put("address", addr);
            input_addrs.push_back(std::make_pair("", input_addr));

            // add input address
            if(!addr.empty()) {
                vec_ip_addr.push_back(addr);
            }
        }
        tx_item.push_back(std::make_pair("inputs", input_addrs));
        
        // set outputs content
        pt::ptree pt_outputs;
        for(auto& op : tx.outputs) {
            pt::ptree pt_output;
            std::string addr="";
			
			auto address = payment_address::extract(op.script);
			if (address)
				addr = address.encoded();
			if(blockchain.get_account_address(auth_.name, addr))
				pt_output.put("own", true);
			else
				pt_output.put("own", false);
            pt_output.put("address", addr);
            pt_output.put("etp-value", op.value);
            //pt_output.add_child("attachment", prop_list(op.attach_data));
			////////////////////////////////////////////////////////////
			auto attach_data = op.attach_data;
			pt::ptree tree;
			if(attach_data.get_type() == ETP_TYPE) {
				tree.put("type", "etp");
			} else if(attach_data.get_type() == ASSET_TYPE) {
				auto asset_info = boost::get<bc::chain::asset>(attach_data.get_attach());
				if(asset_info.get_status() == ASSET_DETAIL_TYPE) {
					tree.put("type", "asset-issue");
					auto detail_info = boost::get<bc::chain::asset_detail>(asset_info.get_data());
					tree.put("symbol", detail_info.get_symbol());
					//tree.put("quantity", detail_info.get_maximum_supply());
					tree.put("maximum_supply", detail_info.get_maximum_supply());
					tree.put("decimal_number", detail_info.get_decimal_number());
					tree.put("issuer", detail_info.get_issuer());
					tree.put("address", detail_info.get_address());
					tree.put("description", detail_info.get_description());
				}
				if(asset_info.get_status() == ASSET_TRANSFERABLE_TYPE) {
					tree.put("type", "asset-transfer");
					auto trans_info = boost::get<bc::chain::asset_transfer>(asset_info.get_data());
					tree.put("symbol", trans_info.get_address());
					tree.put("quantity", trans_info.get_quantity());
					auto symbol = trans_info.get_address();
					auto issued_asset = blockchain.get_issued_asset(symbol);
					if(issued_asset)
						tree.put("decimal_number", issued_asset->get_decimal_number());
				}
			} else if(attach_data.get_type() == MESSAGE_TYPE) {
				tree.put("type", "message");
				auto msg_info = boost::get<bc::chain::blockchain_message>(attach_data.get_attach());
				tree.put("content", msg_info.get_content());
			} else {
				tree.put("type", "unknown business");
			}
            pt_output.add_child("attachment", tree);
			////////////////////////////////////////////////////////////
			
            pt_outputs.push_back(std::make_pair("", pt_output));
            
            // add output address
            if(!addr.empty())
            	vec_op_addr.push_back(addr);
        }
        tx_item.push_back(std::make_pair("outputs", pt_outputs));
        
        // set tx direction
        // 1. receive check
        auto pos = std::find_if(vec_ip_addr.begin(), vec_ip_addr.end(), [&](const std::string& i){
                return blockchain.get_account_address(auth_.name, i) != nullptr;
                });
        
        if (pos == vec_ip_addr.end()){
            tx_item.put("direction", "receive");
        }
        // 2. transfer check
        #if 0
        auto is_ip_intern = true;
        auto is_op_intern = true;

        if(vec_ip_addr.empty())
            is_ip_intern = false;
        for(auto& each : vec_ip_addr) {
            if(!blockchain.get_account_address(auth_.name, each))
                is_ip_intern = false;
        }
        
        for(auto& each : vec_op_addr) {
            if(!blockchain.get_account_address(auth_.name, each))
                is_op_intern = false;
        }
        
        if (is_ip_intern && is_ip_intern){
            tx_item.put("direction", "transfer");
        }
        #endif
        // 3. all address clear
        vec_ip_addr.clear();
        vec_op_addr.clear();
		balances.push_back(std::make_pair("", tx_item));
    }
    aroot.add_child("transactions", balances);
    pt::write_json(output, aroot);

    return console_result::okay;
}
} // namespace commands
} // namespace explorer
} // namespace libbitcoin

