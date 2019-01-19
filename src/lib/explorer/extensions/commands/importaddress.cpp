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

#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/json_helper.hpp>
#include <metaverse/explorer/extensions/commands/importaddress.hpp>
#include <metaverse/explorer/extensions/account_info.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/bitcoin/formats/base_16.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {
namespace fs = boost::filesystem;
using namespace bc::explorer::config;
using namespace bc::chain;

/************************ importkeyfile *************************/

console_result importaddress::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto account = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    account_script acc_script;

    data_chunk redeem_data;
    //1. script
    if (blockchain.is_script_address(argument_.script)) {

        const std::string& p2sh_address = argument_.script;
        acc_script.set_address(p2sh_address);

    } else if (decode_base16(redeem_data, argument_.script)) {
        bc::chain::script redeem_script;
        if (!redeem_script.from_data(redeem_data, false, bc::chain::script::parse_mode::strict)) {
            throw redeem_script_data_exception{"error occured when parse redeem script data."};
        }

        const wallet::payment_address address(redeem_script, wallet::payment_address::mainnet_p2sh);
        const std::string p2sh_address = address.encoded(); // pay address

        acc_script.set_address(p2sh_address);
        acc_script.set_script(redeem_data);

        jv_output["script"] = redeem_script.to_string( chain::script_context::all_enabled );
    } else {
        throw redeem_script_data_exception{"Unexpect \"SCRIPT\" argument format. Only p2sh address or hex-encoded bitcoin script is supported."};
    }

    if (account->is_script_exist(acc_script)) {
        throw multisig_exist_exception{"address[" + acc_script.get_address() + "] already exist."};
    }

    {
        acc_script.set_description(option_.description);

        // change account type
        account->set_type(account_type::script_);

        // create account address
        auto account_address = std::make_shared<bc::chain::account_address>();
        account_address->set_name(auth_.name);
        account_address->set_address(acc_script.get_address());

        // update account and multisig account
        account_address->set_status(account_address_status::script_addr);

        account->set_script(acc_script);

        // store them
        blockchain.store_account(account);
        blockchain.store_account_address(account_address);
    }

    jv_output["address"] = acc_script.get_address();
    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

