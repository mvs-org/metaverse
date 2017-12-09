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
#include <metaverse/explorer/extensions/commands/createasset.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <boost/algorithm/string.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."
/************************ createasset *************************/
static std::vector<std::string> forbidden_str {
        "hujintao",
        "wenjiabao",
        "wjb",
        "xijinping",
        "xjp",
        "tankman",
        "liusi",
        "vpn",
        "64memo",
        "gfw",
        "freedom",
        "freechina",
        "likeqiang",
        "zhouyongkang",
        "lichangchun",
        "wubangguo",
        "heguoqiang",
        "jiangzemin",
        "jzm",
        "fuck",
        "shit",
        "198964",
        "64",
        "gongchandang",
        "gcd",
        "tugong",
        "communism",
        "falungong",
        "communist",
        "party",
        "ccp",
        "cpc",
        "hongzhi",
        "lihongzhi",
        "lhz",
        "dajiyuan",
        "zangdu",
        "dalai",
        "minzhu",
        "China",
        "Chinese",
        "taiwan",
        "SHABI",
        "penis",
        "j8",
        "Islam",
        "allha",
        "USD",
        "CNY",
        "EUR",
        "AUD",
        "GBP",
        "CHF",
        "ETP",
        "currency",
        "asset",
        "balance",
        "exchange",
        "token",
        "BUY",
        "SELL",
        "ASK",
        "BID",
        "ZEN."
};

void validate(boost::any& v,
              const std::vector<std::string>& values,
              non_negative_uint64*, int)
{
    using namespace boost::program_options;
    validators::check_first_occurrence(v);

    std::string const& s = validators::get_single_string(values);
    if (s[0] == '-') {
        throw argument_legality_exception{"volume must not be negative number."};
    }
    //v = lexical_cast<unsigned long long>(s);
    v = boost::any(non_negative_uint64 { boost::lexical_cast<uint64_t>(s) } );
}

console_result createasset::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    // maybe throw
    blockchain.uppercase_symbol(option_.symbol);

    for(auto& each : forbidden_str) {
        if (boost::starts_with(option_.symbol, boost::to_upper_copy(each)))
            throw asset_symbol_name_exception{"invalid symbol and can not begin with " + boost::to_upper_copy(each)};
    }
    
    auto ret = blockchain.is_asset_exist(option_.symbol);
    if(ret) 
        throw asset_symbol_existed_exception{"asset symbol is already exist, please use another one"};
    if (option_.symbol.length() > ASSET_DETAIL_SYMBOL_FIX_SIZE)
        throw asset_symbol_length_exception{"asset symbol length must be less than 64."};
    if (option_.description.length() > ASSET_DETAIL_DESCRIPTION_FIX_SIZE)
        throw asset_description_length_exception{"asset description length must be less than 64."};
    if (auth_.name.length() > 64) // maybe will be remove later
        throw account_length_exception{"asset issue(account name) length must be less than 64."};
    if (option_.decimal_number > 19u)
        throw asset_amount_exception{"asset decimal number must less than 20."};

    if(!option_.maximum_supply.volume) 
        throw argument_legality_exception{"volume must not be zero."};

    auto acc = std::make_shared<asset_detail>();
    acc->set_symbol(option_.symbol);
    acc->set_maximum_supply(option_.maximum_supply.volume);
    //acc->set_maximum_supply(volume);
    acc->set_decimal_number(static_cast<uint8_t>(option_.decimal_number));
    //acc->set_asset_type(asset_detail::asset_detail_type::created); 
    acc->set_issuer(auth_.name);
    acc->set_description(option_.description);
    
    blockchain.store_account_asset(acc);

    //output<<option_.symbol<<" created at local, you can issue it.";
    
    pt::ptree aroot;
    pt::ptree asset_data;
    asset_data.put("symbol", acc->get_symbol());
    asset_data.put("maximum-supply", acc->get_maximum_supply());
    asset_data.put("decimal_number", acc->get_decimal_number());
    asset_data.put("issuer", acc->get_issuer());
    asset_data.put("address", acc->get_address());
    asset_data.put("description", acc->get_description());
    //asset_data.put("status", "issued");
    aroot.push_back(std::make_pair("asset", asset_data));
        
    pt::write_json(output, aroot);
    
    return console_result::okay;
}
} // namespace commands
} // namespace explorer
} // namespace libbitcoin

