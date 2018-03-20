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
#include <metaverse/explorer/extensions/commands/createdid.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <boost/algorithm/string.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;
/************************ createdid *************************/
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
        "ZEN.",
        "DID"
};

void validate(boost::any& v,
              const std::vector<std::string>& values,
              positive_uint64*, int)
{
    using namespace boost::program_options;
    validators::check_first_occurrence(v);

    std::string const& s = validators::get_single_string(values);
    if (s[0] == '-') {
        throw argument_legality_exception{"volume must not be negative number."};
    }
    //v = lexical_cast<unsigned long long>(s);
    v = boost::any(positive_uint64 { boost::lexical_cast<uint64_t>(s) } );
}

console_result createdid::invoke (Json::Value& jv_output,
         libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    if (option_.symbol.length() == 0){
        option_.symbol = auth_.name;
    }
    // maybe throw
    blockchain.uppercase_symbol(option_.symbol);

    for(auto& each : forbidden_str) {
        if (boost::starts_with(option_.symbol, boost::to_upper_copy(each)))
            throw did_symbol_name_exception{"invalid symbol and can not begin with " + boost::to_upper_copy(each)};
    }
    
    auto ret = blockchain.is_did_exist(option_.symbol);
    if(ret) 
        throw did_symbol_existed_exception{"did symbol is already exist, please use another one"};
    if (option_.symbol.length() > DID_DETAIL_SYMBOL_FIX_SIZE)
        throw did_symbol_length_exception{"did symbol length must be less than 64."};
    if (option_.description.length() > DID_DETAIL_DESCRIPTION_FIX_SIZE)
        throw did_description_length_exception{"did description length must be less than 64."};
    if (auth_.name.length() > 64) // maybe will be remove later
        throw account_length_exception{"did issue(account name) length must be less than 64."};
    

    auto acc = std::make_shared<did_detail>();
    acc->set_symbol(option_.symbol);
    acc->set_issuer(auth_.name);
    acc->set_description(option_.description);
    
    blockchain.store_account_did(acc);

    //output<<option_.symbol<<" created at local, you can issue it.";
    
    auto& aroot = jv_output;
    Json::Value did_data;
    did_data["symbol"] = acc->get_symbol();
    did_data["issuer"] = acc->get_issuer();
    did_data["description"] = acc->get_description();
    aroot["did"] = did_data;

    
    return console_result::okay;
}
} // namespace commands
} // namespace explorer
} // namespace libbitcoin

