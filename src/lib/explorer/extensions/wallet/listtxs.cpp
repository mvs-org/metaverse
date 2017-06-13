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

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."

/************************ listtxs *************************/

console_result listtxs::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    using namespace libbitcoin::config; // for hash256
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if (!argument_.address.empty() && !blockchain.is_valid_address(argument_.address))
        throw std::logic_error{"invalid address parameter!"};
    
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
            throw std::logic_error{"nullptr for address list"};
        
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
                throw std::logic_error{"timestamp is invalid format(eg : 123:456)!"};
            }
            uint32_t start, end;
            deserialize(start, tokens[0], true);
            deserialize(end, tokens[1], true);

            auto pvaddr = blockchain.get_account_addresses(auth_.name);
            if(!pvaddr) 
                throw std::logic_error{"nullptr for address list"};

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
				throw std::logic_error{"invalid height option!"};
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
    const char* wallet[2]{"fetch-tx", nullptr};
    std::stringstream sout;
    std::istringstream sin; 
    std::vector<std::string> vec_ip_addr; // input addr
    std::vector<std::string> vec_op_addr; // output addr

    for (auto& elem: *sh_tx_hash) {
        sout.str("");
        wallet[1] = elem.hash_.c_str();
        
        try {
            if(console_result::okay != dispatch_command(2, wallet + 0, sin, sout, sout))
                continue;
        }catch(std::exception& e){
            log::info("listtxs")<<sout.str();
            log::info("listtxs")<<e.what();
            continue;
        }catch(...){
            log::info("listtxs")<<sout.str();
            continue;
        }
        pt::ptree tx;
        sin.str(sout.str());
        pt::read_json(sin, tx);

        auto tx_hash = tx.get<std::string>("transaction.hash");
        auto inputs = tx.get_child("transaction.inputs");
        auto outputs = tx.get_child("transaction.outputs");
        // not found, try next 
        if ((inputs.size() == 0) || (outputs.size() == 0)) {
            continue;
        }

        // found, then push_back
        pt::ptree tx_item;
        for (auto& each: *sh_txs){
            if( each.hash.compare(tx_hash) != 0 )
                continue;

            tx_item.put("hash", each.hash);
            tx_item.put("height", each.height);
            tx_item.put("timestamp", each.timestamp);
            tx_item.put("direction", "send");

            // set inputs content
            pt::ptree input_addrs;
            for(auto& input : inputs) {
                pt::ptree input_addr;
                std::string addr="";
                try {
                    addr = input.second.get<std::string>("address");
                } catch(...){
                    log::info("listtxs no input address!");
                }
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
            for(auto& op : outputs) {
                pt::ptree pt_output;
				if(blockchain.get_account_address(auth_.name, op.second.get<std::string>("address")))
	                pt_output.put("own", true);
				else
                	pt_output.put("own", false);
                pt_output.put("address", op.second.get<std::string>("address"));
                pt_output.put("etp-value", op.second.get<uint64_t>("value"));
                //pt_output.add_child("attachment", op.second.get<pt::ptree>("attachment"));
                pt_output.add_child("attachment", op.second.get_child("attachment"));
                pt_outputs.push_back(std::make_pair("", pt_output));
                
                // add output address
                vec_op_addr.push_back(op.second.get<std::string>("address"));
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
        }
        balances.push_back(std::make_pair("", tx_item));
    }
    
    aroot.add_child("transactions", balances);
    pt::write_json(output, aroot);

    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

