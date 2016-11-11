/**
 * Copyright (c) 2011-2015 mvs developers 
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


#include <bitcoin/explorer/command.hpp>
#include <functional>
#include <memory>    
#include <string>    
#include <array>     
#include <bitcoin/explorer/command.hpp>                
#include <bitcoin/explorer/command_extension.hpp>      
#include <bitcoin/explorer/command_extension_func.hpp>

using namespace std;

namespace libbitcoin {
namespace explorer {

using namespace commands;

void broadcast_extension(const function<void(shared_ptr<command>)> func)
{
    func(make_shared<stop>());
    func(make_shared<getinfo>());
    func(make_shared<getpeerinfo>());
    func(make_shared<ping>());
    func(make_shared<addnode>());
    func(make_shared<getmininginfo>());
    func(make_shared<backupwallet>());
    func(make_shared<importwallet>());
    func(make_shared<dumpprivkey>());
    func(make_shared<importprivkey>());
    func(make_shared<switchaccout>());
    func(make_shared<getnewaccount>());
    func(make_shared<getaccount>());
    func(make_shared<getaddresses>());
    func(make_shared<getnewaddress>());
    func(make_shared<getblock>());
    func(make_shared<signmessage>());
    func(make_shared<verifymessage>());
    func(make_shared<createmultisig>());
    func(make_shared<addmultisigaddress>());
    func(make_shared<validateaddress>());
    func(make_shared<getbalance>());
    func(make_shared<getaddressbalance>());
    func(make_shared<getaccountbalance>());
    func(make_shared<getaddresstransaction>());
    func(make_shared<listaddresstransactions>());
    func(make_shared<listaccounttransactions>());
    func(make_shared<send>());
    func(make_shared<sendfrom>());
    func(make_shared<sendmessage>());
    func(make_shared<sendmessagefrom>());
    func(make_shared<startmining>());
    func(make_shared<stopmining>());
    func(make_shared<listassets>());
    func(make_shared<issue>());
    func(make_shared<issuefrom>());
    func(make_shared<issuemore>());
    func(make_shared<issuemorefrom>());
    func(make_shared<sendasset>());
    func(make_shared<sendassetfrom>());
    func(make_shared<sendwithmsg>());
    func(make_shared<sendwithmsgfrom>());
    func(make_shared<settxfee>());

}

shared_ptr<command> find_extension(const string& symbol)
{
    if (symbol == stop::symbol())
        return make_shared<stop>();
    if (symbol == getinfo::symbol())
        return make_shared<getinfo>();
    if (symbol == getpeerinfo::symbol())
        return make_shared<getpeerinfo>();
    if (symbol == ping::symbol())
        return make_shared<ping>();
    if (symbol == addnode::symbol())
        return make_shared<addnode>();
    if (symbol == getmininginfo::symbol())
        return make_shared<getmininginfo>();
    if (symbol == backupwallet::symbol())
        return make_shared<backupwallet>();
    if (symbol == importwallet::symbol())
        return make_shared<importwallet>();
    if (symbol == dumpprivkey::symbol())
        return make_shared<dumpprivkey>();
    if (symbol == importprivkey::symbol())
        return make_shared<importprivkey>();
    if (symbol == switchaccout::symbol())
        return make_shared<switchaccout>();
    if (symbol == getnewaccount::symbol())
        return make_shared<getnewaccount>();
    if (symbol == getaccount::symbol())
        return make_shared<getaccount>();
    if (symbol == getaddresses::symbol())
        return make_shared<getaddresses>();
    if (symbol == getnewaddress::symbol())
        return make_shared<getnewaddress>();
    if (symbol == getblock::symbol())
        return make_shared<getblock>();
    if (symbol == signmessage::symbol())
        return make_shared<signmessage>();
    if (symbol == verifymessage::symbol())
        return make_shared<verifymessage>();
    if (symbol == createmultisig::symbol())
        return make_shared<createmultisig>();
    if (symbol == addmultisigaddress::symbol())
        return make_shared<addmultisigaddress>();
    if (symbol == validateaddress::symbol())
        return make_shared<validateaddress>();
    if (symbol == getbalance::symbol())
        return make_shared<getbalance>();
    if (symbol == getaddressbalance::symbol())
        return make_shared<getaddressbalance>();
    if (symbol == getaccountbalance::symbol())
        return make_shared<getaccountbalance>();
    if (symbol == getaddresstransaction::symbol())
        return make_shared<getaddresstransaction>();
    if (symbol == listaddresstransactions::symbol())
        return make_shared<listaddresstransactions>();
    if (symbol == listaccounttransactions::symbol())
        return make_shared<listaccounttransactions>();
    if (symbol == send::symbol())
        return make_shared<send>();
    if (symbol == sendfrom::symbol())
        return make_shared<sendfrom>();
    if (symbol == sendmessage::symbol())
        return make_shared<sendmessage>();
    if (symbol == sendmessagefrom::symbol())
        return make_shared<sendmessagefrom>();
    if (symbol == startmining::symbol())
        return make_shared<startmining>();
    if (symbol == stopmining::symbol())
        return make_shared<stopmining>();
    if (symbol == listassets::symbol())
        return make_shared<listassets>();
    if (symbol == issue::symbol())
        return make_shared<issue>();
    if (symbol == issuefrom::symbol())
        return make_shared<issuefrom>();
    if (symbol == issuemore::symbol())
        return make_shared<issuemore>();
    if (symbol == issuemorefrom::symbol())
        return make_shared<issuemorefrom>();
    if (symbol == sendasset::symbol())
        return make_shared<sendasset>();
    if (symbol == sendassetfrom::symbol())
        return make_shared<sendassetfrom>();
    if (symbol == sendwithmsg::symbol())
        return make_shared<sendwithmsg>();
    if (symbol == sendwithmsgfrom::symbol())
        return make_shared<sendwithmsgfrom>();
    if (symbol == settxfee::symbol())
        return make_shared<settxfee>();
    return nullptr;

}

std::string formerly_extension(const string& former)
{
    return "";

}

} // namespace explorer
} // namespace libbitcoin

