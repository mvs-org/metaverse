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
#include <metaverse/explorer/extensions/commands/deleteasset.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <boost/algorithm/string.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

/************************ deleteasset *************************/

console_result deleteasset::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    // maybe throw
    blockchain.uppercase_symbol(option_.symbol);

    if(blockchain.get_issued_asset(option_.symbol))
        throw asset_issued_not_delete{"Cannot delete asset " + option_.symbol + " which has been issued."};

    std::promise<code> p;
    std::vector<libbitcoin::blockchain::transaction_pool::transaction_ptr> txs;
    blockchain.pool().fetch([&txs, &p](const code& ec, const std::vector<libbitcoin::blockchain::transaction_pool::transaction_ptr>& tx){
        if (!ec)
        {
            txs = tx;
        }
        p.set_value(ec);
    });
    p.get_future().get();

    for(auto& tx : txs)
    {
        for(auto& output : tx->outputs)
        {
            if(output.is_asset_issue() && output.get_asset_symbol() == option_.symbol)    
                throw asset_issued_not_delete{"Cannot delete asset " + option_.symbol + " which has been issued."};
        }
    }

    std::vector<business_address_asset> assets = *blockchain.get_account_unissued_assets(auth_.name);
    bool found = false;
    for(auto it = assets.begin(); it != assets.end(); ++it)
    {
        if(it->detail.get_symbol() == option_.symbol)
        {
            if(blockchain.delete_account_asset(auth_.name) == console_result::failure)
                throw asset_delete_fail{"asset " + option_.symbol + " delete fail."};

            assets.erase(it);
            for(auto asset : assets)
            {
                blockchain.store_account_asset(asset.detail);
            }

            found = true;
            break;
        }
    }

    if(!found)
    {
        throw asset_notfound_exception{"asset " + option_.symbol + " is not existed or is not belong to " + auth_.name + "."};
    }

    Json::Value result;
    result.put("symbol", option_.symbol);
    result.put("operate", "delete");
    result.put("result", "success");
    pt::write_json(output, result);

    return console_result::okay;
}
} // namespace commands
} // namespace explorer
} // namespace libbitcoin
