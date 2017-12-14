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


#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/listtxs.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


class BC_API tx_block_info
{
public:
    tx_block_info(uint64_t height, uint32_t timestamp, hash_digest hash):
        height_(height), timestamp_(timestamp), hash_(hash)
    {}
    uint64_t get_height() {
        return height_;
    }
    uint32_t get_timestamp() {
        return timestamp_;
    }
    hash_digest get_hash(){
        return hash_;
    }
    bool operator<(const tx_block_info & rinfo) const
    {
        return hash_ < const_cast<tx_block_info&>(rinfo).get_hash();
    }
    bool operator==(const tx_block_info& rinfo) const
    {
        return hash_ == const_cast<tx_block_info&>(rinfo).get_hash();
    }
    
private:
    uint64_t height_;
    uint32_t timestamp_;
    hash_digest hash_;
};


/************************ listtxs *************************/

console_result listtxs::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    using namespace libbitcoin::config; // for hash256
    auto& blockchain = node.chain_impl(); 
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    // address option check
    if (!argument_.address.empty() && !blockchain.is_valid_address(argument_.address))
        throw address_invalid_exception{"invalid address parameter!"};
    // height check
    if(option_.height.first() 
        && option_.height.second() 
        && (option_.height.first() >= option_.height.second())) {
        throw block_height_exception{"invalid height option!"};
    }
    // symbol check
    if(!argument_.symbol.empty()) {
        blockchain.uppercase_symbol(argument_.symbol);
        if(!blockchain.get_issued_asset(argument_.symbol))
            throw asset_symbol_notfound_exception{argument_.symbol + std::string(" not exist!")};
    }

    Json::Value aroot;
    Json::Value balances;
    
    auto sort_by_height = [](const tx_block_info &lhs, const tx_block_info &rhs)->bool { 
        return const_cast<tx_block_info&>(lhs).get_height() > const_cast<tx_block_info&>(rhs).get_height(); 
    };
    
    auto sh_txs = std::make_shared<std::vector<tx_block_info>>();
    auto sh_addr_vec = std::make_shared<std::vector<std::string>>();

    // collect address
    if(argument_.address.empty()) { 
        auto pvaddr = blockchain.get_account_addresses(auth_.name);
        if(!pvaddr) 
            throw address_invalid_exception{"nullptr for address list"};
        
        for (auto& elem: *pvaddr) {
            sh_addr_vec->push_back(elem.get_address());
        }
    } else { // address exist in command
        sh_addr_vec->push_back(argument_.address);
    }

    // scan all addresses business record
    for (auto& each: *sh_addr_vec) {
        auto sh_vec = blockchain.get_address_business_record(each, argument_.symbol,
                option_.height.first(), option_.height.second(), 0, 0);
        for(auto& elem : *sh_vec)
            sh_txs->push_back(tx_block_info(elem.height, elem.data.get_timestamp(), elem.point.hash));
    }
    std::sort (sh_txs->begin(), sh_txs->end());
    sh_txs->erase(std::unique(sh_txs->begin(), sh_txs->end()), sh_txs->end());
    std::sort (sh_txs->begin(), sh_txs->end(), sort_by_height);

    // page limit & page index paramenter check
    if(!argument_.index) 
        throw argument_legality_exception{"page index parameter must not be zero"};    
    if(!argument_.limit) 
        throw argument_legality_exception{"page record limit parameter must not be zero"};    
    if(argument_.limit > 100)
        throw argument_legality_exception{"page record limit must not be bigger than 100."};

    uint64_t start, end, total_page, tx_count;
    if(argument_.index && argument_.limit) {
        start = (argument_.index - 1)*argument_.limit;
        end = (argument_.index)*argument_.limit;
        if(start >= sh_txs->size() || !sh_txs->size())
            throw argument_legality_exception{"no record in this page"};

        total_page = sh_txs->size() % argument_.limit ? (sh_txs->size()/argument_.limit + 1) : (sh_txs->size()/argument_.limit);
        tx_count = end >=sh_txs->size()? (sh_txs->size() - start) : argument_.limit ;
        
    } else if(!argument_.index && !argument_.limit) { // all tx records
        start = 0;
        tx_count = sh_txs->size();
        argument_.index = 1;
        total_page = 1;
    } else {
        throw argument_legality_exception{"invalid limit or index parameter"};
    }

    // sort by height
    std::vector<tx_block_info> result(sh_txs->begin() + start, sh_txs->begin() + start + tx_count);

    // fetch tx according its hash
    std::vector<std::string> vec_ip_addr; // input addr
    std::vector<std::string> vec_op_addr; // output addr
    chain::transaction tx;
    uint64_t tx_height;
    //hash_digest trans_hash;
    for (auto& each: result){
        //decode_hash(trans_hash, each.hash);
        if(!blockchain.get_transaction(each.get_hash(), tx, tx_height))
            continue;
        
        Json::Value tx_item;
        tx_item.put("hash", encode_hash(each.get_hash()));
        tx_item.put("height", each.get_height());
        tx_item.put("timestamp", each.get_timestamp());
        tx_item.put("direction", "send");

        // set inputs content
        Json::Value input_addrs;
        for(auto& input : tx.inputs) {
            Json::Value input_addr;
            std::string addr="";
            
            auto script_address = payment_address::extract(input.script);
            if (script_address)
                addr = script_address.encoded();

            input_addr.put("address", addr);
            input_addr.put("script", script(input.script).to_string(1));
            input_addrs.push_back(std::make_pair("", input_addr));

            // add input address
            if(!addr.empty()) {
                vec_ip_addr.push_back(addr);
            }
        }
        tx_item.push_back(std::make_pair("inputs", input_addrs));
        
        // set outputs content
        Json::Value pt_outputs;
        for(auto& op : tx.outputs) {
            Json::Value pt_output;
            std::string addr="";
            
            auto address = payment_address::extract(op.script);
            if (address)
                addr = address.encoded();
            if(blockchain.get_account_address(auth_.name, addr))
                pt_output.put("own", true);
            else
                pt_output.put("own", false);
            pt_output.put("address", addr);
            pt_output.put("script", script(op.script).to_string(1));
            uint64_t lock_height = 0;
            if(chain::operation::is_pay_key_hash_with_lock_height_pattern(op.script.operations))
                lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(op.script.operations);
            pt_output.put("locked_height_range", lock_height);
            pt_output.put("etp-value", op.value);
            //pt_output.add_child("attachment", json_helper().prop_list(op.attach_data));
            ////////////////////////////////////////////////////////////
            auto attach_data = op.attach_data;
            Json::Value tree;
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
    aroot.put("total_page", total_page);
    aroot.put("current_page", argument_.index);
    aroot.put("transaction_count", tx_count);
    aroot.add_child("transactions", balances);
    pt::write_json(output, aroot);

    return console_result::okay;
}
} // namespace commands
} // namespace explorer
} // namespace libbitcoin
