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
#include <metaverse/explorer/extensions/commands/deposit.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/command_assistant.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/explorer/extensions/base_helper.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

console_result deposit::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    attachment attach;
    std::string addr;

    if (argument_.address.empty()) {
        auto pvaddr = blockchain.get_account_addresses(auth_.name);
        if(!pvaddr || pvaddr->empty())
            throw address_list_nullptr_exception{"nullptr for address list"};

        addr = get_random_payment_address(pvaddr, blockchain);
    }
    else {
        addr = get_address(argument_.address, attach, false, blockchain);
    }

    if (argument_.deposit != 7 && argument_.deposit != 30
        && argument_.deposit != 90 && argument_.deposit != 182
        && argument_.deposit != 365)
    {
        throw account_deposit_period_exception{"deposit must be one in [7, 30, 90, 182, 365]."};
    }

    // receiver
    std::vector<receiver_record> receiver{
        {addr, "", argument_.amount, 0, utxo_attach_type::deposit, attach}
    };
    auto deposit_helper = depositing_etp(*this, blockchain, std::move(auth_.name), std::move(auth_.auth),
            std::move(addr), std::move(receiver), argument_.deposit, argument_.fee);

    deposit_helper.exec();

    // json output
    auto tx = deposit_helper.get_transaction();
     jv_output =  config::json_helper(get_api_version()).prop_tree(tx, true);

    return console_result::okay;
}

} // namespace commands
} // namespace explorer
} // namespace libbitcoin

