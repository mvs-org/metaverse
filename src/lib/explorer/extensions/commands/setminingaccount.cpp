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


#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/setminingaccount.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ setminingaccount *************************/

console_result setminingaccount::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto& miner = node.miner();

    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    auto address = get_address(argument_.payment_address, blockchain);

    auto sp_account_address = blockchain.get_account_address(auth_.name, address);
    if (!sp_account_address) {
        throw address_dismatch_account_exception{
            "did/address does not match account. " + argument_.payment_address};
    }

    auto& symbol = argument_.asset_symbol;
    if (!symbol.empty()) {
        if (!miner.set_mining_asset_symbol(symbol)) {
            throw argument_legality_exception{"asset " + symbol + " can not be mined."};
        }
    }

    miner.set_miner_payment_address(wallet::payment_address(address));

    std::string text = "Address [" + address + "] setted.";
    if (!symbol.empty()) {
        text += " Also mining asset " + symbol;
    }
    jv_output = text;

    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

