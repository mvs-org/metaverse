/**
 * Copyright (c) 2016-2020 mvs developers
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
#include <metaverse/explorer/extensions/commands/getblock.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ getblock *************************/

console_result getblock::invoke(Json::Value& jv_output,
                                libbitcoin::server::server_node& node)
{
    auto json = option_.json;
    auto tx_json = option_.tx_json;

    // uint64_t max length
    if (argument_.hash_or_height.size() < 18) {

        // fetch_block via height
        auto block_height = std::stoull(argument_.hash_or_height);

        std::promise<code> p;
        auto& blockchain = node.chain_impl();
        blockchain.fetch_block(block_height, [&p, &jv_output, json, tx_json, this](const code & ec, chain::block::ptr block) {
            if (ec) {
                p.set_value(ec);
                return;
            }

            if (json) {
                jv_output =  config::json_helper(get_api_version()).prop_tree(*block, json, tx_json);
            }
            else {
                jv_output =  config::json_helper(get_api_version()).prop_tree(*block, false, false);
            }
            p.set_value(error::success);
        });

        auto result = p.get_future().get();
        if (result) {
            throw block_height_get_exception{ result.message() };
        }
    }

    else {
        // fetch_block via hash
        bc::config::hash256 block_hash(argument_.hash_or_height);

        std::promise<code> p;
        auto& blockchain = node.chain_impl();
        blockchain.fetch_block(block_hash, [&p, &jv_output, json, tx_json, this](const code & ec, chain::block::ptr block) {
            if (ec) {
                p.set_value(ec);
                return;
            }

            if (json) {
                jv_output =  config::json_helper(get_api_version()).prop_tree(*block, json, tx_json);
            }
            else {
                jv_output =  config::json_helper(get_api_version()).prop_tree(*block, false, false);
            }
            p.set_value(error::success);
        });

        auto result = p.get_future().get();
        if (result) {
            throw block_height_get_exception{ result.message() };
        }
    }

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

