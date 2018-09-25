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
#include <metaverse/explorer/extensions/commands/sendassetfrom.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

console_result sendassetfrom::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);
    blockchain.uppercase_symbol(argument_.symbol);

    // check asset symbol
    check_asset_symbol(argument_.symbol);

    attachment attach;
    std::string from_address = get_address(argument_.from, attach, true, blockchain);
    std::string to_address = get_address(argument_.to, attach, false, blockchain);
    std::string change_address = get_address(option_.change, blockchain);

    if (argument_.amount <= 0) {
        throw asset_amount_exception("invalid amount parameter!");
    }

    check_message(option_.memo);

    // receiver
    utxo_attach_type attach_type = option_.attenuation_model_param.empty()
        ? utxo_attach_type::asset_transfer : utxo_attach_type::asset_attenuation_transfer;
    std::vector<receiver_record> receiver{
        {to_address, argument_.symbol, 0, argument_.amount, attach_type, attach}
    };
    auto send_helper = sending_asset(*this, blockchain,
        std::move(auth_.name), std::move(auth_.auth),
        std::move(from_address), std::move(argument_.symbol),
        std::move(option_.attenuation_model_param),
        std::move(receiver), option_.fee,
        std::move(option_.memo), std::move(change_address));

    send_helper.exec();

    // json output
    auto tx = send_helper.get_transaction();
    jv_output = config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

