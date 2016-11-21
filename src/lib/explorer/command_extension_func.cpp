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
    func(make_shared<start>());
    func(make_shared<setadmin>());
    func(make_shared<getinfo>());
    func(make_shared<getpeerinfo>());
    func(make_shared<ping>());
    func(make_shared<addnode>());
    func(make_shared<getmininginfo>());
    func(make_shared<backupwallet>());
    func(make_shared<importwallet>());
    func(make_shared<lockwallet>());
    func(make_shared<backupaccount>());
    func(make_shared<importaccount>());
    func(make_shared<switchaccount>());
    func(make_shared<listaccounts>());
    func(make_shared<getnewaccount>());
    func(make_shared<getaccount>());
    func(make_shared<lockaccount>());
    func(make_shared<setaccountinfo>());
    func(make_shared<listaddresses>());
    func(make_shared<getnewaddress>());
    func(make_shared<getaddress>());
    func(make_shared<getblock>());
    func(make_shared<signmessage>());
    func(make_shared<verifymessage>());
    func(make_shared<createmultisig>());
    func(make_shared<addmultisigaddress>());
    func(make_shared<validateaddress>());
    func(make_shared<listbalances>());
    func(make_shared<getbalance>());
    func(make_shared<getaddressbalance>());
    func(make_shared<getaccountbalance>());
    func(make_shared<listtxs>());
    func(make_shared<gettx>());
    func(make_shared<getaddresstx>());
    func(make_shared<getaccounttx>());
    func(make_shared<send>());
    func(make_shared<sendmore>());
    func(make_shared<sendfrom>());
    func(make_shared<sendwithmsg>());
    func(make_shared<sendwithmsgfrom>());
    func(make_shared<listassets>());
    func(make_shared<getasset>());
    func(make_shared<getaddressasset>());
    func(make_shared<getaccountasset>());
    func(make_shared<createasset>());
    func(make_shared<issue>());
    func(make_shared<issuefrom>());
    func(make_shared<issuemore>());
    func(make_shared<issuemorefrom>());
    func(make_shared<getdid>());
    func(make_shared<setdid>());
    func(make_shared<sendwithdid>());
    func(make_shared<settxfee>());

}

shared_ptr<command> find_extension(const string& symbol)
{
    if (symbol == stop::symbol())
        return make_shared<stop>();
    if (symbol == start::symbol())
        return make_shared<start>();
    if (symbol == setadmin::symbol())
        return make_shared<setadmin>();
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
    if (symbol == lockwallet::symbol())
        return make_shared<lockwallet>();
    if (symbol == backupaccount::symbol())
        return make_shared<backupaccount>();
    if (symbol == importaccount::symbol())
        return make_shared<importaccount>();
    if (symbol == switchaccount::symbol())
        return make_shared<switchaccount>();
    if (symbol == listaccounts::symbol())
        return make_shared<listaccounts>();
    if (symbol == getnewaccount::symbol())
        return make_shared<getnewaccount>();
    if (symbol == getaccount::symbol())
        return make_shared<getaccount>();
    if (symbol == lockaccount::symbol())
        return make_shared<lockaccount>();
    if (symbol == setaccountinfo::symbol())
        return make_shared<setaccountinfo>();
    if (symbol == listaddresses::symbol())
        return make_shared<listaddresses>();
    if (symbol == getnewaddress::symbol())
        return make_shared<getnewaddress>();
    if (symbol == getaddress::symbol())
        return make_shared<getaddress>();
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
    if (symbol == listbalances::symbol())
        return make_shared<listbalances>();
    if (symbol == getbalance::symbol())
        return make_shared<getbalance>();
    if (symbol == getaddressbalance::symbol())
        return make_shared<getaddressbalance>();
    if (symbol == getaccountbalance::symbol())
        return make_shared<getaccountbalance>();
    if (symbol == listtxs::symbol())
        return make_shared<listtxs>();
    if (symbol == gettx::symbol())
        return make_shared<gettx>();
    if (symbol == getaddresstx::symbol())
        return make_shared<getaddresstx>();
    if (symbol == getaccounttx::symbol())
        return make_shared<getaccounttx>();
    if (symbol == send::symbol())
        return make_shared<send>();
    if (symbol == sendmore::symbol())
        return make_shared<sendmore>();
    if (symbol == sendfrom::symbol())
        return make_shared<sendfrom>();
    if (symbol == sendwithmsg::symbol())
        return make_shared<sendwithmsg>();
    if (symbol == sendwithmsgfrom::symbol())
        return make_shared<sendwithmsgfrom>();
    if (symbol == listassets::symbol())
        return make_shared<listassets>();
    if (symbol == getasset::symbol())
        return make_shared<getasset>();
    if (symbol == getaddressasset::symbol())
        return make_shared<getaddressasset>();
    if (symbol == getaccountasset::symbol())
        return make_shared<getaccountasset>();
    if (symbol == createasset::symbol())
        return make_shared<createasset>();
    if (symbol == issue::symbol())
        return make_shared<issue>();
    if (symbol == issuefrom::symbol())
        return make_shared<issuefrom>();
    if (symbol == issuemore::symbol())
        return make_shared<issuemore>();
    if (symbol == issuemorefrom::symbol())
        return make_shared<issuemorefrom>();
    if (symbol == getdid::symbol())
        return make_shared<getdid>();
    if (symbol == setdid::symbol())
        return make_shared<setdid>();
    if (symbol == sendwithdid::symbol())
        return make_shared<sendwithdid>();
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

