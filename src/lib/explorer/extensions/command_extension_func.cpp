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


#include <functional>
#include <memory>    
#include <string>    
#include <array>     

#include <metaverse/explorer/command.hpp>                
#include <metaverse/explorer/dispatch.hpp>
#include <metaverse/explorer/extensions/command_extension.hpp>   
#include <metaverse/explorer/extensions/command_extension_func.hpp>

#include <metaverse/explorer/extensions/commands/shutdown.hpp>
#include <metaverse/explorer/extensions/commands/stopmining.hpp>
#include <metaverse/explorer/extensions/commands/startmining.hpp>
#include <metaverse/explorer/extensions/commands/getinfo.hpp>
#include <metaverse/explorer/extensions/commands/getheight.hpp>
#include <metaverse/explorer/extensions/commands/getpeerinfo.hpp>
#include <metaverse/explorer/extensions/commands/getaddressetp.hpp>
#include <metaverse/explorer/extensions/commands/addnode.hpp>
#include <metaverse/explorer/extensions/commands/getmininginfo.hpp>
#include <metaverse/explorer/extensions/commands/getblockheader.hpp>
#include <metaverse/explorer/extensions/commands/fetchheaderext.hpp>
#include <metaverse/explorer/extensions/commands/gettx.hpp>
#include <metaverse/explorer/extensions/commands/dumpkeyfile.hpp>
#include <metaverse/explorer/extensions/commands/importkeyfile.hpp>
#include <metaverse/explorer/extensions/commands/importaccount.hpp>
#include <metaverse/explorer/extensions/commands/getnewaccount.hpp>
#include <metaverse/explorer/extensions/commands/getaccount.hpp>
#include <metaverse/explorer/extensions/commands/deleteaccount.hpp>
#include <metaverse/explorer/extensions/commands/listaddresses.hpp>
#include <metaverse/explorer/extensions/commands/getnewaddress.hpp>
#include <metaverse/explorer/extensions/commands/getblock.hpp>
#include <metaverse/explorer/extensions/commands/validateaddress.hpp>
#include <metaverse/explorer/extensions/commands/listbalances.hpp>
#include <metaverse/explorer/extensions/commands/getbalance.hpp>
#include <metaverse/explorer/extensions/commands/listtxs.hpp>
#include <metaverse/explorer/extensions/commands/deposit.hpp>
#include <metaverse/explorer/extensions/commands/send.hpp>
#include <metaverse/explorer/extensions/commands/sendmore.hpp>
#include <metaverse/explorer/extensions/commands/sendfrom.hpp>
#include <metaverse/explorer/extensions/commands/listassets.hpp>
#include <metaverse/explorer/extensions/commands/getasset.hpp>
#include <metaverse/explorer/extensions/commands/getaddressasset.hpp>
#include <metaverse/explorer/extensions/commands/getaccountasset.hpp>
#include <metaverse/explorer/extensions/commands/createasset.hpp>
#include <metaverse/explorer/extensions/commands/createdid.hpp>
#include <metaverse/explorer/extensions/commands/deletelocalasset.hpp>
#include <metaverse/explorer/extensions/commands/issue.hpp>
#include <metaverse/explorer/extensions/commands/issuefrom.hpp>
#include <metaverse/explorer/extensions/commands/sendasset.hpp>
#include <metaverse/explorer/extensions/commands/sendassetfrom.hpp>
#include <metaverse/explorer/extensions/commands/getwork.hpp>
#include <metaverse/explorer/extensions/commands/submitwork.hpp>
#include <metaverse/explorer/extensions/commands/setminingaccount.hpp>
#include <metaverse/explorer/extensions/commands/changepasswd.hpp>
#include <metaverse/explorer/extensions/commands/getmemorypool.hpp>
#include <metaverse/explorer/extensions/commands/createmultisigtx.hpp>
#include <metaverse/explorer/extensions/commands/createrawtx.hpp>
#include <metaverse/explorer/extensions/commands/decoderawtx.hpp>
#include <metaverse/explorer/extensions/commands/deletemultisig.hpp>
#include <metaverse/explorer/extensions/commands/getnewmultisig.hpp>
#include <metaverse/explorer/extensions/commands/getpublickey.hpp>
#include <metaverse/explorer/extensions/commands/listmultisig.hpp>
#include <metaverse/explorer/extensions/commands/sendrawtx.hpp>
#include <metaverse/explorer/extensions/commands/sendwithmsg.hpp>
#include <metaverse/explorer/extensions/commands/sendwithmsgfrom.hpp>
#include <metaverse/explorer/extensions/commands/signmultisigtx.hpp>
#include <metaverse/explorer/extensions/commands/signrawtx.hpp>


namespace libbitcoin {
namespace explorer {


void broadcast_extension(const function<void(shared_ptr<command>)> func)
{
    using namespace std;
    using namespace commands;

    // account
    func(make_shared<getnewaccount>());
    func(make_shared<getnewaddress>());
    func(make_shared<getaccount>());
    func(make_shared<deleteaccount>());
    func(make_shared<listaddresses>());
    func(make_shared<dumpkeyfile>());
    func(make_shared<importkeyfile>());
    func(make_shared<importaccount>());
    func(make_shared<changepasswd>());

    // wallet
    func(make_shared<getheight>());
    func(make_shared<getblock>());
    func(make_shared<getblockheader>());
    func(make_shared<fetchheaderext>());
    func(make_shared<shutdown>());
    func(make_shared<startmining>());
    func(make_shared<stopmining>());
    func(make_shared<setminingaccount>());
    func(make_shared<getmininginfo>());
    func(make_shared<getinfo>());
    func(make_shared<getpeerinfo>());
    func(make_shared<getaddressetp>());
    func(make_shared<addnode>());
    func(make_shared<gettx>());
    func(make_shared<getwork>());
    func(make_shared<submitwork>());
    func(make_shared<getmemorypool>());

    // etp
    func(make_shared<validateaddress>());
    func(make_shared<listbalances>());
    func(make_shared<getbalance>());
    func(make_shared<listtxs>());
    func(make_shared<deposit>());
    func(make_shared<send>());
    func(make_shared<sendmore>());
    func(make_shared<sendfrom>());

    // asset
    func(make_shared<listassets>());
    func(make_shared<getasset>());
    func(make_shared<getaddressasset>());
    func(make_shared<getaccountasset>());
    func(make_shared<createasset>());
    func(make_shared<deletelocalasset>());
    func(make_shared<issue>());
    func(make_shared<issuefrom>());
    func(make_shared<sendasset>());
    func(make_shared<sendassetfrom>());

    // multi-sig
    func(make_shared<getpublickey>());
    func(make_shared<createmultisigtx>());
    func(make_shared<getnewmultisig>());
    func(make_shared<listmultisig>());
    func(make_shared<deletemultisig>());
    func(make_shared<signmultisigtx>());

    // raw
    func(make_shared<createrawtx>());
    func(make_shared<decoderawtx>());
    func(make_shared<signrawtx>());
    func(make_shared<sendrawtx>());

    //did
    func(make_shared<createdid>());
}

shared_ptr<command> find_extension(const string& symbol)
{
    using namespace std;
    using namespace commands;
    if (symbol == stopmining::symbol() || symbol == "stop")
        return make_shared<stopmining>();
    if (symbol == startmining::symbol() || symbol == "start")
        return make_shared<startmining>();
    if (symbol == getinfo::symbol())
        return make_shared<getinfo>();
    if (symbol == getheight::symbol())
        return make_shared<getheight>();
    if (symbol == "fetch-height")
        return make_shared<getheight>(symbol);
    if (symbol == getpeerinfo::symbol())
        return make_shared<getpeerinfo>();
    if (symbol == getaddressetp::symbol() || symbol == "fetch-balance")
        return make_shared<getaddressetp>();
    if (symbol == addnode::symbol())
        return make_shared<addnode>();
    if (symbol == getmininginfo::symbol())
        return make_shared<getmininginfo>();
    if (symbol == gettx::symbol() || symbol == "gettransaction")
        return make_shared<gettx>();
    if (symbol == "fetch-tx")
        return make_shared<gettx>(symbol);
    if (symbol == dumpkeyfile::symbol() || symbol == "exportaccountasfile")
        return make_shared<dumpkeyfile>();
    if (symbol == importkeyfile::symbol() || symbol == "importaccountfromfile")
        return make_shared<importkeyfile>();
    if (symbol == importaccount::symbol())
        return make_shared<importaccount>();
    if (symbol == getnewaccount::symbol())
        return make_shared<getnewaccount>();
    if (symbol == getaccount::symbol())
        return make_shared<getaccount>();
    if (symbol == deleteaccount::symbol())
        return make_shared<deleteaccount>();
    if (symbol == listaddresses::symbol())
        return make_shared<listaddresses>();
    if (symbol == getnewaddress::symbol())
        return make_shared<getnewaddress>();
    if (symbol == getpublickey::symbol())
        return make_shared<getpublickey>();
    if (symbol == getblock::symbol())
        return make_shared<getblock>();
    if (symbol == validateaddress::symbol())
        return make_shared<validateaddress>();
    if (symbol == listbalances::symbol())
        return make_shared<listbalances>();
    if (symbol == getbalance::symbol())
        return make_shared<getbalance>();
    if (symbol == "getbestblockhash")
        return make_shared<getblockheader>(symbol);
    if (symbol == getblockheader::symbol() || symbol == "fetch-header" || symbol == "getbestblockheader")
        return make_shared<getblockheader>();
    if (symbol == fetchheaderext::symbol())
        return make_shared<fetchheaderext>();
    if (symbol == listtxs::symbol())
        return make_shared<listtxs>();
    if (symbol == deposit::symbol())
        return make_shared<deposit>();
    if (symbol == send::symbol())
        return make_shared<send>();
    if (symbol == sendmore::symbol())
        return make_shared<sendmore>();
    if (symbol == sendfrom::symbol())
        return make_shared<sendfrom>();
    if (symbol == listassets::symbol())
        return make_shared<listassets>();
    if (symbol == getasset::symbol())
        return make_shared<getasset>();
    if (symbol == getaccountasset::symbol())
        return make_shared<getaccountasset>();
    if (symbol == createasset::symbol())
        return make_shared<createasset>();
    if (symbol == deletelocalasset::symbol() || symbol == "deleteasset" )
        return make_shared<deletelocalasset>();
    if (symbol == issue::symbol())
        return make_shared<issue>();
    if (symbol == issuefrom::symbol())
        return make_shared<issuefrom>();
    if (symbol == sendasset::symbol())
        return make_shared<sendasset>();
    if (symbol == sendassetfrom::symbol())
        return make_shared<sendassetfrom>();
    if (symbol == getwork::symbol())
        return make_shared<getwork>();
    if (symbol == submitwork::symbol())
        return make_shared<submitwork>();
    if (symbol == setminingaccount::symbol())
        return make_shared<setminingaccount>();
    if (symbol == changepasswd::symbol())
        return make_shared<changepasswd>();
    if (symbol == getnewmultisig::symbol())
        return make_shared<getnewmultisig>();
    if (symbol == listmultisig::symbol())
        return make_shared<listmultisig>();
    if (symbol == deletemultisig::symbol())
        return make_shared<deletemultisig>();
    if (symbol == createmultisigtx::symbol())
        return make_shared<createmultisigtx>();
    if (symbol == signmultisigtx::symbol())
        return make_shared<signmultisigtx>();
    if (symbol == createrawtx::symbol())
        return make_shared<createrawtx>();
    if (symbol == decoderawtx::symbol())
        return make_shared<decoderawtx>();
    if (symbol == signrawtx::symbol())
        return make_shared<signrawtx>();
    if (symbol == sendrawtx::symbol())
        return make_shared<sendrawtx>();
    if (symbol == issuefrom::symbol())
        return make_shared<issuefrom>();
    if (symbol == sendassetfrom::symbol())
        return make_shared<sendassetfrom>();
    if (symbol == shutdown::symbol())
        return make_shared<shutdown>();
    if (symbol == getmemorypool::symbol())
        return make_shared<getmemorypool>();
    if (symbol == getaddressasset::symbol())
        return make_shared<getaddressasset>();
    if (symbol == createdid::symbol())
        return make_shared<createdid>();
    return nullptr;
}

std::string formerly_extension(const string& former)
{
    return "";
}

} // namespace explorer
} // namespace libbitcoin

