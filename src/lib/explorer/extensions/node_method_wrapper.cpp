/**
 * Copyright (c) 2016-2021 mvs developers
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

#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/node_method_wrapper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

// ----------------------------- checker methods ------------------------------

void administrator_required_checker(bc::server::server_node& node,
        const std::string& name, const std::string& auth)
{
    auto& blockchain = node.chain_impl();
    // administrator_required option is true
    if (node.server_settings().administrator_required) {
        if(!blockchain.is_admin_account(name))
            throw account_authority_exception{"Administrator name must be [administerator],incorrect vocabulary @_@."};

        blockchain.is_account_passwd_valid(name, auth);
    }
}

// ----------------------------- qurey methods ------------------------------
uint64_t get_last_height(bc::server::server_node& node)
{
    uint64_t last_height;
    std::promise<code> p;

    auto& blockchain = node.chain_impl();
    blockchain.fetch_last_height([&p, &last_height](const code& ec, uint64_t height){
            if(ec){
                    p.set_value(ec);
                    return;
            }

            last_height = height;

            p.set_value(error::success);
    });

    auto result = p.get_future().get();
    if(result){
            throw block_height_get_exception{result.message()};
    }

    return last_height;
}

uint32_t get_connections_count(bc::server::server_node& node){
    uint32_t ret;

    node.connections_ptr()->count([&ret](size_t count){
        ret = count;
    });

    return ret;
}


} //commands
} // explorer
} // libbitcoin
