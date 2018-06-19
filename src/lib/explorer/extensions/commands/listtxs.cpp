/**
 * Copyright (c) 2016-2018 mvs developers
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

#include <metaverse/explorer/json_helper.hpp>
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/listtxs.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

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

console_result listtxs::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    using namespace libbitcoin::config; // for hash256
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
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

    auto& aroot = jv_output;
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
        throw argument_legality_exception{"page index parameter cannot be zero"};
    if(!argument_.limit)
        throw argument_legality_exception{"page record limit parameter cannot be zero"};
    if(argument_.limit > 100)
        throw argument_legality_exception{"page record limit cannot be bigger than 100."};

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

    auto json_helper = config::json_helper(get_api_version());

    // sort by height
    std::vector<tx_block_info> result(sh_txs->begin() + start, sh_txs->begin() + start + tx_count);

    // fetch tx according its hash
    std::vector<std::string> vec_ip_addr; // input addr
    chain::transaction tx;
    uint64_t tx_height;
    //hash_digest trans_hash;
    for (auto& each: result){
        //decode_hash(trans_hash, each.hash);
        if(!blockchain.get_transaction(each.get_hash(), tx, tx_height))
            continue;

        Json::Value tx_item;
        tx_item["hash"] = encode_hash(each.get_hash());
        if (get_api_version() == 1) {
            tx_item["height"] += each.get_height();
            tx_item["timestamp"] += each.get_timestamp();
        } else {
            tx_item["height"] = each.get_height();
            tx_item["timestamp"] = each.get_timestamp();
        }
        tx_item["direction"] = "send";

        tx_item["remark"] = blockchain.get_account_remark(acc->get_name(), each.get_hash());

        // set inputs content
        Json::Value input_addrs;
        for(auto& input : tx.inputs) {
            Json::Value input_addr;

            auto&& script_address = payment_address::extract(input.script);
            if (script_address) {
                auto&& temp_addr = script_address.encoded();
                input_addr["address"] = temp_addr;
                // add input address
                vec_ip_addr.push_back(temp_addr);
            } else {
                // empty input address : coin base tx;
                if (get_api_version() == 1)
                    input_addr["address"] = "";
                else
                    input_addr["address"] = Json::nullValue;
            }

            input_addr["script"] = input.script.to_string(1);
            input_addrs.append(input_addr);

        }

        if (get_api_version() == 1 && input_addrs.isNull()) { // compatible for v1
            tx_item["inputs"] = "";
        } else {
            tx_item["inputs"] = input_addrs;
        }

        // set outputs content
        Json::Value pt_outputs;
        uint64_t lock_height = 0;
        for(auto& op : tx.outputs) {
            Json::Value pt_output;

            auto&& address = payment_address::extract(op.script);
            if (address) {
                auto&& temp_addr = address.encoded();
                pt_output["address"] = temp_addr;
                auto ret = blockchain.get_account_address(auth_.name, temp_addr);
                if(get_api_version() == 1)
                    pt_output["own"] = ret ? "true" : "false";
                else
                    pt_output["own"] = ret ? true : false;

            } else {
                // empty output address ? unbelievable.
                if(get_api_version() == 1)
                    pt_output["address"] = "";
                else
                    pt_output["address"] = Json::nullValue;
            }

            pt_output["script"] = op.script.to_string(1);
            lock_height = 0;
            if(chain::operation::is_pay_key_hash_with_lock_height_pattern(op.script.operations))
                lock_height = chain::operation::get_lock_height_from_pay_key_hash_with_lock_height(op.script.operations);

            if (get_api_version() == 1) {
                pt_output["locked_height_range"] += lock_height;
                pt_output["etp-value"] += op.value;
            } else {
                pt_output["locked_height_range"] = lock_height;
                pt_output["etp-value"] = op.value;
            }

            if (chain::operation::is_pay_key_hash_with_attenuation_model_pattern(op.script.operations)) {
                const auto& model_param = op.get_attenuation_model_param();
                pt_output["attenuation_model_param"] = json_helper.prop_attenuation_model_param(model_param);
            }

            auto attach_data = op.attach_data;
            Json::Value tree = json_helper.prop_list(attach_data);

            if (attach_data.get_type() == ASSET_TYPE) {
                auto asset_info = boost::get<bc::chain::asset>(attach_data.get_attach());
                if (asset_info.get_status() == ASSET_TRANSFERABLE_TYPE) {
                    // asset_transfer dose not contain decimal_number message,
                    // so we get decimal_number from the issued asset with the same symbol.
                    auto symbol = tree["symbol"].asString();
                    auto issued_asset = blockchain.get_issued_asset(symbol);

                    if (issued_asset) {
                        if (get_api_version() == 1) {
                            tree["decimal_number"] += issued_asset->get_decimal_number();
                        } else {
                            tree["decimal_number"] = issued_asset->get_decimal_number();
                        }
                    }
                }
            }

            pt_output["attachment"] = tree;
            ////////////////////////////////////////////////////////////

            pt_outputs.append(pt_output);

        }

        if (get_api_version() == 1 && pt_outputs.isNull()) { // compatible for v1
            tx_item["outputs"] = "";
        } else {
            tx_item["outputs"] = pt_outputs;
        }

        // set tx direction
        // 1. receive check
        auto pos = std::find_if(vec_ip_addr.begin(), vec_ip_addr.end(), [&](const std::string& i){
                return blockchain.get_account_address(auth_.name, i) != nullptr;
                });

        if (pos == vec_ip_addr.end()){
            tx_item["direction"] = "receive";
        }
        // 3. all address clear
        vec_ip_addr.clear();
        balances.append(tx_item);
    }

    if (get_api_version() == 1) {
        aroot["total_page"] += total_page;
        aroot["current_page"] += argument_.index;
        aroot["transaction_count"] += tx_count;
    } else {
        aroot["total_page"] = total_page;
        aroot["current_page"] = argument_.index;
        aroot["transaction_count"] = tx_count;
    }

    if (get_api_version() == 1 && balances.isNull()) { // compatible for v1
        aroot["transactions"] = "";
    } else {
        aroot["transactions"] = balances;
    }

    return console_result::okay;
}
} // namespace commands
} // namespace explorer
} // namespace libbitcoin
