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

#include <jsoncpp/json/json.h>
#include <metaverse/explorer/extensions/base_helper.hpp>
#include <metaverse/explorer/extensions/commands/getaddressetp.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::explorer::config;

/************************ getaddressetp *************************/

Json::Value to_json_value(const deposited_balance& balance)
{
    Json::Value json_balance;
    json_balance["address"] = balance.address;
    json_balance["deposited_balance"] = balance.balance;
    json_balance["bonus_balance"] = balance.bonus;
    json_balance["deposited_height"] = balance.deposited_height;
    json_balance["expiration_height"] = balance.expiration_height;
    json_balance["tx_hash"] = balance.tx_hash;
    return json_balance;
}

Json::Value to_json_value(const balances& addr_balance, uint8_t api_version)
{
    Json::Value json_balance;
    if (api_version == 1) {
        json_balance["confirmed"] = std::to_string(addr_balance.confirmed_balance);
        json_balance["received"]  = std::to_string(addr_balance.total_received);
        json_balance["unspent"]   = std::to_string(addr_balance.unspent_balance);
        json_balance["frozen"]    = std::to_string(addr_balance.frozen_balance);
    }
    else {
        json_balance["confirmed"] = addr_balance.confirmed_balance;
        json_balance["received"]  = addr_balance.total_received;
        json_balance["unspent"]   = addr_balance.unspent_balance;
        json_balance["frozen"]    = addr_balance.frozen_balance;
        json_balance["available"] = (addr_balance.unspent_balance - addr_balance.frozen_balance);
    }
    return json_balance;
}

Json::Value to_json_value(const utxo_balance& balance)
{
    Json::Value json_balance;
    json_balance["available"] = (balance.unspent_balance - balance.frozen_balance);
    json_balance["balance"] = balance.unspent_balance;
    json_balance["frozen"] = balance.frozen_balance;
    json_balance["utxo_block"] = balance.output_height;
    json_balance["utxo_hash"] = balance.output_hash;
    json_balance["utxo_index"] = balance.output_index;
    return json_balance;
}

console_result getaddressetp::invoke(Json::Value& jv_output,
                                     libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto&& address = get_address(argument_.address, blockchain);

    wallet::payment_address waddr(address);

    Json::Value balances;

    if (option_.deposited) {
        auto deposited_balances = std::make_shared<deposited_balance::list>();
        sync_fetch_deposited_balance(waddr, blockchain, deposited_balances);
        for (const auto& balance : *deposited_balances) {
            balances.append(to_json_value(balance));
        }
    }
    else if (!option_.utxo) {
        commands::balances addr_balance{0, 0, 0, 0};
        sync_fetchbalance(waddr, blockchain, addr_balance);
        balances = to_json_value(addr_balance, get_api_version());
        balances["address"] = address;
    }
    else {
        auto utxo_balances = std::make_shared<utxo_balance::list>();
        sync_fetchbalance(waddr, blockchain, utxo_balances);
        for (const auto& balance : *utxo_balances) {
            balances.append(to_json_value(balance));
        }
    }

    if (get_api_version() <= 2) {
        jv_output["balance"] = balances;
    }
    else {
        jv_output = balances;
    }

    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

