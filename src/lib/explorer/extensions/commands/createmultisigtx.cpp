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


#include <metaverse/explorer/json_helper.hpp>
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/commands/createmultisigtx.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

using namespace bc::explorer::config;

console_result createmultisigtx::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto acc = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    if(!blockchain.is_valid_address(argument_.from))
        throw fromaddress_invalid_exception{"invalid from address!"};

    auto addr = bc::wallet::payment_address(argument_.from);
    if(addr.version() != bc::wallet::payment_address::mainnet_p2sh) // for multisig address
        throw fromaddress_invalid_exception{"from address is not script address."};
    if(!blockchain.is_valid_address(argument_.to))
        throw toaddress_invalid_exception{"invalid to address!"};

    account_multisig acc_multisig;
    if(!(acc->get_multisig_by_address(acc_multisig, argument_.from)))
        throw multisig_notfound_exception{"from address multisig record not found."};

    std::shared_ptr<base_transfer_common> sp_send_helper;

    // receiver
    std::vector<receiver_record> receiver;

    auto type = static_cast<utxo_attach_type>(option_.type);
    switch (type) {
        case utxo_attach_type::etp:
        {
            receiver.push_back({argument_.to, "", argument_.amount, 0, type, attachment()});
        }
        break;
        case utxo_attach_type::asset_transfer:
        {
            blockchain.uppercase_symbol(option_.symbol);
            check_asset_symbol(option_.symbol);
            receiver.push_back({argument_.to, option_.symbol, 0, argument_.amount, type, attachment()});
        }
        break;
        default:
        {
            throw argument_legality_exception{"invalid transaction type."};
        }
        break;
    }

    sp_send_helper = std::make_shared<sending_multisig_tx>(*this, blockchain,
        std::move(auth_.name), std::move(auth_.auth),
        std::move(argument_.from), std::move(receiver),
        argument_.fee, acc_multisig, std::move(option_.symbol));

    sp_send_helper->exec();

    // json output
    auto&& tx = sp_send_helper->get_transaction();

    std::ostringstream config_tx;
    config_tx << config::transaction(tx);
    jv_output = config_tx.str();

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

