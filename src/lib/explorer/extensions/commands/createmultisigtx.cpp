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
#include <metaverse/explorer/extensions/commands/createmultisigtx.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

namespace pt = boost::property_tree;

console_result createmultisigtx::invoke (std::ostream& output,
        std::ostream& cerr, libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if(!blockchain.is_valid_address(argument_.from)) 
        throw fromaddress_invalid_exception{"invalid from address!"};
    
    auto addr = bc::wallet::payment_address(argument_.from);
    if(addr.version() != 0x05) // for multisig address
        throw fromaddress_invalid_exception{"from address is not script address."};
    if(!blockchain.is_valid_address(argument_.to)) 
        throw toaddress_invalid_exception{"invalid to address!"};
    
    account_multisig acc_multisig;
    if(!(acc->get_multisig_by_address(acc_multisig, argument_.from)))
        throw multisig_notfound_exception{"from address multisig record not found."};
    // receiver
    std::vector<receiver_record> receiver{
        {argument_.to, "", argument_.amount, 0, utxo_attach_type::etp, attachment()}  
    };
    auto send_helper = sending_multisig_etp(*this, blockchain, std::move(auth_.name), std::move(auth_.auth), 
            std::move(argument_.from), std::move(receiver), argument_.fee, 
            acc_multisig);
    
    send_helper.exec();

    // json output
    auto tx = send_helper.get_transaction();

    //pt::write_json(output, config::prop_tree(tx, true));
    //output << "raw tx content" << std::endl << config::transaction(tx);
    output << config::transaction(tx);
    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

