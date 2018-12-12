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
#include <metaverse/explorer/extensions/commands/importcontract.hpp>
#include <metaverse/explorer/extensions/exception.hpp>
#include <metaverse/bitcoin/chain/script/operation.hpp>
#include <metaverse/bitcoin/formats/base_16.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {

/************************ importcontract *************************/
console_result importcontract::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto account = blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

    int need_role = 0;
    std::string contract;
    if (argument_.templ == "delegate") {
        need_role = 2; // Alice and Bob are required
        contract = "dup hash160 [ %1% ] equal if "
                     "checksig "
                   "else "
                     "[ bfffffff ] checksequenceverify dup hash160 [ %2% ] equalverify checksig "
                   "endif";
    } else {
        throw argument_dismatch_exception{"template [" + argument_.templ + "] not exist."};
    }

    boost::format fmter(contract);
    for (int i=0; i < MAX_ROLES; ++i) {
        if (i < need_role) {
            if (option_.roles[i].empty()) {
                throw argument_legality_exception{"the role is missing."};
            }

            if (!blockchain.is_payment_address(option_.roles[i])) {
                throw address_invalid_exception{"the address [" + option_.roles[i] + "] is not a valid payment address."};
            }

            wallet::payment_address addr{option_.roles[i]};
            fmter % encode_base16(addr.hash());
        } else {
            if (!option_.roles[i].empty()) {
                throw argument_legality_exception{"the role is not required."};
            }
        }
    }

    jv_output["contract"] = fmter.str();

    script sc()

    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

