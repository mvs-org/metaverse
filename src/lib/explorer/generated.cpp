/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
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
#include <metaverse/explorer/generated.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <metaverse/explorer/command.hpp>
#include <metaverse/explorer/extensions/command_extension.hpp>
#include <metaverse/explorer/extensions/command_extension_func.hpp>

/********* GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY **********/

using namespace po;
using namespace std;
using namespace boost::filesystem;

namespace libbitcoin {
namespace explorer {
using namespace commands;

void broadcast(const function<void(shared_ptr<command>)> func, std::ostream& os)
{
    os <<"== original commands ==\r\n";

    func(make_shared<help>());
    func(make_shared<settings>());
    func(make_shared<fetch_history>());
    func(make_shared<send_tx>());
    func(make_shared<tx_decode>());
    func(make_shared<validate_tx>());
    func(make_shared<stealth_decode>());
    func(make_shared<stealth_encode>());
    func(make_shared<stealth_public>());
    func(make_shared<stealth_secret>());
    func(make_shared<stealth_shared>());

    os <<"\r\n== extension commands ==\r\n";

    broadcast_extension(func, os);
}

shared_ptr<command> find(const string& symbol)
{
    if (symbol == help::symbol())
        return make_shared<help>();
    if (symbol == send_tx::symbol())
        return make_shared<send_tx>();
    if (symbol == settings::symbol())
        return make_shared<settings>();
    if (symbol == fetch_history::symbol())
        return make_shared<fetch_history>();
    if (symbol == stealth_decode::symbol())
        return make_shared<stealth_decode>();
    if (symbol == stealth_encode::symbol())
        return make_shared<stealth_encode>();
    if (symbol == stealth_public::symbol())
        return make_shared<stealth_public>();
    if (symbol == stealth_secret::symbol())
        return make_shared<stealth_secret>();
    if (symbol == stealth_shared::symbol())
        return make_shared<stealth_shared>();
    if (symbol == tx_decode::symbol())
        return make_shared<tx_decode>();
    if (symbol == validate_tx::symbol())
        return make_shared<validate_tx>();

    return find_extension(symbol);
}

std::string formerly(const string& former)
{
    if (former == send_tx::formerly())
        return send_tx::symbol();
    if (former == stealth_decode::formerly())
        return stealth_decode::symbol();
    if (former == stealth_public::formerly())
        return stealth_public::symbol();
    if (former == stealth_secret::formerly())
        return stealth_secret::symbol();
    if (former == validate_tx::formerly())
        return validate_tx::symbol();

    return "";
}

} // namespace explorer
} // namespace libbitcoin
