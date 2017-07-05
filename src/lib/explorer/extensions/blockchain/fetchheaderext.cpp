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
#include <metaverse/explorer/extensions/blockchain/fetchheaderext.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

#define IN_DEVELOPING "this command is in deliberation, or replace it with original command."

/************************ fetchheaderext *************************/

console_result fetchheaderext::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    using namespace libbitcoin::config; // for hash256
    
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if(argument_.number.empty())
        throw query_block_exception{"Block number or earliest, latest, pending is needed"};

    chain::header block_header;

    auto& miner = node.miner();
    auto ret = miner.get_block_header(block_header, argument_.number);

    pt::ptree aroot;
    pt::ptree tree;
    
    if(ret) {
        tree.put("bits", block_header.bits);
        tree.put("hash", hash256(block_header.hash()));
        tree.put("merkle_tree_hash", hash256(block_header.merkle));
        tree.put("nonce", block_header.nonce);
        tree.put("previous_block_hash", hash256(block_header.previous_block_hash));
        tree.put("time_stamp", block_header.timestamp);
        tree.put("version", block_header.version);
        // wdy added
        tree.put("mixhash", block_header.mixhash);
        tree.put("number", block_header.number);
        tree.put("transaction_count", block_header.transaction_count);
    }
    aroot.push_back(std::make_pair("result", tree));
    pt::write_json(output, aroot);

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

