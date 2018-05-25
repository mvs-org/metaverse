/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse.
 *
 * metaverse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
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
#include <metaverse/bitcoin/wallet/dictionary.hpp>
#include <boost/algorithm/string.hpp>

namespace libbitcoin {
namespace wallet {
namespace symbol {

    const dictionary2 ban_list = {
        "HUJINTAO",
        "WENJIABAO",
        "WJB",
        "XIJINPING",
        "XJP",
        "TANKMAN",
        "LIUSI",
        "VPN",
        "64MEMO",
        "GFW",
        "FREEDOM",
        "FREECHINA",
        "LIKEQIANG",
        "ZHOUYONGKANG",
        "LICHANGCHUN",
        "WUBANGGUO",
        "HEGUOQIANG",
        "JIANGZEMIN",
        "JZM",
        "FUCK",
        "SHIT",
        "198964",
        "GONGCHANDANG",
        "GCD",
        "TUGONG",
        "COMMUNISM",
        "FALUNGONG",
        "COMMUNIST",
        "PARTY",
        "CCP",
        "CPC",
        "HONGZHI",
        "LIHONGZHI",
        "LHZ",
        "DAJIYUAN",
        "ZANGDU",
        "DALAI",
        "MINZHU",
        "CHINA",
        "CHINESE",
        "TAIWAN",
        "SHABI",
        "PENIS",
        "J8",
        "ISLAM",
        "ALLHA",
        "USD",
        "CNY",
        "EUR",
        "AUD",
        "GBP",
        "CHF",
        "ETP",
        "CURRENCY",
        "ASSET",
        "BALANCE",
        "EXCHANGE",
        "TOKEN",
        "BUY",
        "SELL",
        "ASK",
        "BID",
        "DID",
        "ZEN."
    };

    bool is_sensitive(const std::string& symbol) {
        for(auto& each : ban_list) {
            if (symbol.find(each) != std::string::npos) {
               return true;
            }
        }
        return false;
    }

    const std::vector<std::string> forbidden_list = {
        "ETP"
    };

    bool is_forbidden(const std::string& symbol) {
        for(auto& each : forbidden_list) {
            if (symbol == each) {
               return true;
            }
        }
        return false;
    }


} // namespace language
} // namespace wallet
} // namespace libbitcoin
