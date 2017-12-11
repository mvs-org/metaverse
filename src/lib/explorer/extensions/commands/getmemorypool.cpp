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

#include <metaverse/explorer/extensions/commands/getmemorypool.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/node_method_wrapper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;


/************************ getbalance *************************/

console_result getmemorypool::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{

    administrator_required_checker(node, auth_.name, auth_.auth);
    auto json = option_.json;

    using transaction_ptr = message::transaction_message::ptr ;
    auto& blockchain = node.chain_impl();
    std::promise<code> p;
    blockchain.pool().fetch([&output, &p, &json](const code& ec, const std::vector<transaction_ptr>& txs){
        if (ec)
        {
            p.set_value(ec);
            return;
        }
        std::vector<config::transaction> txs1;
        txs1.reserve(txs.size());
        for (auto tp:txs) {
            txs1.push_back(*tp);
        }

        if(json) {
        	pt::write_json(output, config::prop_tree(txs1, true));
        } else {
        	pt::write_json(output, config::prop_tree(txs1, false));
        }
        p.set_value(ec);
    });

    auto result = p.get_future().get();
    if(result){
        throw tx_fetch_exception{result.message()};
    }
    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

