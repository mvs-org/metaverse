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
#include <metaverse/explorer/extensions/commands/getblock.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;


/************************ getblock *************************/

console_result getblock::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto json = argument_.json;
    std::promise<code> p;
    auto& blockchain = node.chain_impl();
    blockchain.fetch_block(argument_.hash, [&p, &output, json](const code& ec, chain::block::ptr block){
            if(ec){
                    p.set_value(ec);
                    return;
            }

            if(json) {
                    pt::write_json(output, config::prop_tree(*block));
            }
            else
            {
                    auto chunck = block->to_data();
                    output << encode_base16(chunck);
            }
            p.set_value(error::success);
    });

    auto result = p.get_future().get();
    if(result){
            throw block_height_get_exception{result.message()};
    }
    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

