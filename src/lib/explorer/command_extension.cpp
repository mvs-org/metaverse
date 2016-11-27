/**
 * Copyright (c) 2016 mvs developers 
 *
 * This file is part of libbitcoin-explorer.
 *
 * libbitcoin-explorer is free software: you can redistribute it and/or
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


#include <bitcoin/explorer/command_extension.hpp>

#include <iostream>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/client.hpp>
#include <bitcoin/explorer/define.hpp>
#include <bitcoin/explorer/callback_state.hpp>
#include <bitcoin/explorer/display.hpp>
#include <bitcoin/explorer/prop_tree.hpp>

#include <bitcoin/explorer/dispatch.hpp>
#include <json/minijson_writer.hpp>
#include <array>

using namespace bc;
using namespace bc::chain;
using namespace bc::client;
using namespace bc::explorer;
using namespace bc::explorer::commands;
using namespace bc::explorer::config;

namespace libbitcoin {
namespace explorer {
namespace commands {


/************************ stop *************************/
console_result stop::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result stop::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ start *************************/
console_result start::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result start::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ setadmin *************************/
console_result setadmin::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result setadmin::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ getinfo *************************/
console_result getinfo::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result getinfo::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ getpeerinfo *************************/
console_result getpeerinfo::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result getpeerinfo::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ ping *************************/
console_result ping::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result ping::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ addnode *************************/
console_result addnode::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result addnode::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ getmininginfo *************************/
console_result getmininginfo::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result getmininginfo::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ backupwallet *************************/
console_result backupwallet::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result backupwallet::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ importwallet *************************/
console_result importwallet::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result importwallet::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ lockwallet *************************/
console_result lockwallet::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result lockwallet::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ backupaccount *************************/
console_result backupaccount::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result backupaccount::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ importaccount *************************/
console_result importaccount::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result importaccount::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ switchaccount *************************/
console_result switchaccount::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result switchaccount::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ listaccounts *************************/
console_result listaccounts::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result listaccounts::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ getnewaccount *************************/
console_result getnewaccount::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result getnewaccount::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    const char* cmds[]{"seed", "mnemonic-new", "mnemonic-to-seed", "hd-new", "hd-to-public"};
    std::ostringstream sout("");
    std::istringstream sin;
    minijson::object_writer json_writer(output);

    auto exec_with = [&](int i){
        sin.str(sout.str());
        sout.str("");
        dispatch_command(1, cmds + i, sin, sout, sout);
    };

    exec_with(0);
    exec_with(1);
    json_writer.write("mnemonic", sout.str());
    blockchain.get_new_account(auth_.name, auth_.auth, sout.str());
    
    exec_with(2);
    exec_with(3);
    exec_with(4);
    json_writer.write("publickey", sout.str());
    json_writer.close();

    return console_result::okay;
}


/************************ getaccount *************************/
console_result getaccount::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result getaccount::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    account_ptr acc = blockchain.get_account_by_name(auth_.name);
    output << acc->to_string();

    return console_result::okay;
}


/************************ lockaccount *************************/
console_result lockaccount::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result lockaccount::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ setaccountinfo *************************/
console_result setaccountinfo::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result setaccountinfo::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ listaddresses *************************/
console_result listaddresses::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result listaddresses::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ getnewaddress *************************/
console_result getnewaddress::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result getnewaddress::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    account_ptr acc = blockchain.get_account_by_name(auth_.name);
    if (acc->mnemonic.empty()) return  console_result::failure;

    const char* cmds[]{"mnemonic-to-seed", "hd-new", "hd-to-ec", "ec-to-public", "ec-to-address"};
    std::ostringstream sout("");
    std::istringstream sin(acc->mnemonic);

    auto exec_with = [&](int i){
        sin.str(sout.str());
        sout.str("");
        return dispatch_command(1, cmds + i, sin, sout, sout);
    };

    dispatch_command(1, cmds, sin, sout, sout);
    exec_with(1);

    std::string argv_index = std::to_string(acc->hd_index);
    const char* hd_private_gen[3] = {"hd-private", "-i", argv_index.c_str()};
    sin.str(sout.str());
    sout.str("");
    dispatch_command(3, hd_private_gen, sin, sout, sout);

    exec_with(2);
    exec_with(3);
    exec_with(4);
    output<<sout.str();


    //acc->hd_index++;

    return console_result::okay;
}


/************************ getaddress *************************/
console_result getaddress::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result getaddress::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ getblock *************************/
console_result getblock::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result getblock::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ signmessage *************************/
console_result signmessage::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result signmessage::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ verifymessage *************************/
console_result verifymessage::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result verifymessage::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ createmultisig *************************/
console_result createmultisig::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result createmultisig::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ addmultisigaddress *************************/
console_result addmultisigaddress::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result addmultisigaddress::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ validateaddress *************************/
console_result validateaddress::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result validateaddress::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ listbalances *************************/
console_result listbalances::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result listbalances::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ getbalance *************************/
console_result getbalance::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result getbalance::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ getaddressbalance *************************/
console_result getaddressbalance::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result getaddressbalance::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ getaccountbalance *************************/
console_result getaccountbalance::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result getaccountbalance::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ listtxs *************************/
console_result listtxs::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result listtxs::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ gettx *************************/
console_result gettx::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result gettx::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ getaddresstx *************************/
console_result getaddresstx::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result getaddresstx::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ getaccounttx *************************/
console_result getaccounttx::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result getaccounttx::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ send *************************/
console_result send::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result send::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ sendmore *************************/
console_result sendmore::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result sendmore::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ sendfrom *************************/
console_result sendfrom::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result sendfrom::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    //todo
    //fetch-tx get outputs
    //input-set 
    //input-sgin
    //input-validate
    //send-tx transacton base16 code
    return console_result::okay;
}


/************************ sendwithmsg *************************/
console_result sendwithmsg::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result sendwithmsg::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ sendwithmsgfrom *************************/
console_result sendwithmsgfrom::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result sendwithmsgfrom::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ listassets *************************/
console_result listassets::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result listassets::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ getasset *************************/
console_result getasset::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result getasset::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ getaddressasset *************************/
console_result getaddressasset::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result getaddressasset::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ getaccountasset *************************/
console_result getaccountasset::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result getaccountasset::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ createasset *************************/
console_result createasset::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result createasset::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    //todo
    //createasset
    //
    return console_result::okay;
}


/************************ issue *************************/
console_result issue::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result issue::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ issuefrom *************************/
console_result issuefrom::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result issuefrom::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    //sendassetfrom - to systemaccount
    return console_result::okay;
}


/************************ issuemore *************************/
console_result issuemore::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result issuemore::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ issuemorefrom *************************/
console_result issuemorefrom::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result issuemorefrom::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ sendasset *************************/
console_result sendasset::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result sendasset::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ sendassetfrom *************************/
console_result sendassetfrom::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result sendassetfrom::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    //attachasset
    //sendfrom
    return console_result::okay;
}


/************************ getdid *************************/
console_result getdid::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result getdid::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ setdid *************************/
console_result setdid::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result setdid::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ sendwithdid *************************/
console_result sendwithdid::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result sendwithdid::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


/************************ settxfee *************************/
console_result settxfee::invoke (std::ostream& output, std::ostream& cerr)
{
    return console_result::okay;
}

console_result settxfee::invoke (std::ostream& output,
        std::ostream& cerr, bc::blockchain::block_chain_impl& blockchain)
{
    return console_result::okay;
}


} // namespace commands
} // namespace explorer
} // namespace libbitcoin

