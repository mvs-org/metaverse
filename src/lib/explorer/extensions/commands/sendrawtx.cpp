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


#include <metaverse/explorer/extensions/commands/sendrawtx.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

console_result sendrawtx::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    // get raw tx
    std::ostringstream buffer;
    pt::write_json(buffer, config::prop_tree(argument_.transaction, true));
    log::trace("sendrawtx=") << buffer.str();
    tx_type tx_ = argument_.transaction;

    // max transfer fee check
    uint64_t inputs_etp_val = 0, outputs_etp_val = tx_.total_output_value();
    if(!blockchain.get_tx_inputs_etp_value(tx_, inputs_etp_val))
        throw tx_validate_exception{std::string("get transaction inputs etp value error!")};
    if((inputs_etp_val - outputs_etp_val) > argument_.fee) //  fee more than max limit etp
        throw tx_validate_exception{std::string("invalid tx fee")};
    if(blockchain.validate_transaction(tx_))
        throw tx_validate_exception{std::string("validate transaction failure")};
    if(blockchain.broadcast_transaction(tx_)) 
        throw tx_broadcast_exception{std::string("broadcast transaction failure")};

    pt::ptree aroot;
    aroot.put("hash", encode_hash(tx_.hash()));
    pt::write_json(output, aroot);
    
    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

