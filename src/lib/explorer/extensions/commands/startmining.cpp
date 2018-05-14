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
#include <metaverse/explorer/extensions/commands/startmining.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>
#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ startmining *************************/

console_result startmining::invoke(Json::Value& jv_output,
    libbitcoin::server::server_node& node)
{
    auto& blockchain = node.chain_impl();
    auto& miner = node.miner();

    uint64_t height;
    uint64_t rate;
    std::string difficulty;
    bool is_solo_mining;
    node.miner().get_state(height, rate, difficulty, is_solo_mining);
    if (is_solo_mining) {
        throw setting_required_exception{"It's mining, please use command <stopmining> to stop the running mining."};
    }

    auto str_addr = option_.address;

    if (str_addr.empty()) {
        Json::Value jv_temp;

        // get new address
        const char* cmds2[]{"getnewaddress", auth_.name.c_str(), auth_.auth.c_str()};

        if (dispatch_command(3, cmds2, jv_temp, node, 2) != console_result::okay) {
            throw address_generate_exception(jv_temp.asString());
        }

        str_addr = jv_temp["addresses"][0].asString();
    } else {
        blockchain.is_account_passwd_valid(auth_.name, auth_.auth);

        if (!blockchain.is_valid_address(str_addr)) {
            throw address_invalid_exception{"invalid address parameter! " + str_addr};
        }

        if (!blockchain.get_account_address(auth_.name, str_addr)) {
            throw address_dismatch_account_exception{"target address does not match account. " + str_addr};
        }
    }

    bc::wallet::payment_address addr(str_addr);

    if (addr.version() == bc::wallet::payment_address::mainnet_p2sh) { // for multisig address
        throw argument_legality_exception{"script address parameter not allowed!"};
    }

    // start
    if (miner.start(addr, option_.number)){
        if (option_.number == 0) {
            jv_output = "solo mining started at " + str_addr;
        } else {
            jv_output = "solo mining started at " + str_addr
                + ", try to mine " + std::to_string(option_.number) + " block(s).";
        }
    } else {
        throw unknown_error_exception{"solo mining startup got error"};
    }

    return console_result::okay;
}



} // namespace commands
} // namespace explorer
} // namespace libbitcoin

