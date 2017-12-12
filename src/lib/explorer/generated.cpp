/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS)
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

    func(make_shared<ec_to_address>());
    func(make_shared<ec_to_public>());
    func(make_shared<fetch_header>());
    func(make_shared<fetch_history>());
    func(make_shared<fetch_stealth>());
    func(make_shared<fetch_tx>());
    func(make_shared<fetch_tx_index>());
    func(make_shared<fetch_utxo>());
    func(make_shared<help>());
    func(make_shared<input_set>());
    func(make_shared<input_sign>());
    func(make_shared<input_validate>());
    func(make_shared<send_tx>());
    func(make_shared<settings>());
    func(make_shared<stealth_decode>());
    func(make_shared<stealth_encode>());
    func(make_shared<stealth_public>());
    func(make_shared<stealth_secret>());
    func(make_shared<stealth_shared>());
    func(make_shared<tx_decode>());
    func(make_shared<tx_encode>());
    func(make_shared<tx_sign>());
    func(make_shared<validate_tx>());

    os <<"\r\n== extension commands ==\r\n";

    broadcast_extension(func);

}

shared_ptr<command> find(const string& symbol)
{
    if (symbol == ec_to_address::symbol())
        return make_shared<ec_to_address>();
    if (symbol == ec_to_public::symbol())
        return make_shared<ec_to_public>();
    if (symbol == fetch_header::symbol())
        return make_shared<fetch_header>();
    if (symbol == fetch_history::symbol())
        return make_shared<fetch_history>();
    if (symbol == fetch_stealth::symbol())
        return make_shared<fetch_stealth>();
    if (symbol == fetch_tx::symbol())
        return make_shared<fetch_tx>();
    if (symbol == fetch_tx_index::symbol())
        return make_shared<fetch_tx_index>();
    if (symbol == fetch_utxo::symbol())
        return make_shared<fetch_utxo>();
    if (symbol == help::symbol())
        return make_shared<help>();
    if (symbol == input_set::symbol())
        return make_shared<input_set>();
    if (symbol == input_sign::symbol())
        return make_shared<input_sign>();
    if (symbol == input_validate::symbol())
        return make_shared<input_validate>();
    if (symbol == send_tx::symbol())
        return make_shared<send_tx>();
    if (symbol == settings::symbol())
        return make_shared<settings>();
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
    if (symbol == tx_encode::symbol())
        return make_shared<tx_encode>();
    if (symbol == tx_sign::symbol())
        return make_shared<tx_sign>();
    if (symbol == validate_tx::symbol())
        return make_shared<validate_tx>();

    return find_extension(symbol);
}

std::string formerly(const string& former)
{
    if (former == ec_to_address::formerly())
        return ec_to_address::symbol();
    if (former == ec_to_public::formerly())
        return ec_to_public::symbol();
    if (former == fetch_tx::formerly())
        return fetch_tx::symbol();
    if (former == fetch_tx_index::formerly())
        return fetch_tx_index::symbol();
    if (former == fetch_utxo::formerly())
        return fetch_utxo::symbol();
    if (former == input_set::formerly())
        return input_set::symbol();
    if (former == input_sign::formerly())
        return input_sign::symbol();
    if (former == input_validate::formerly())
        return input_validate::symbol();
    if (former == send_tx::formerly())
        return send_tx::symbol();
    if (former == stealth_decode::formerly())
        return stealth_decode::symbol();
    if (former == stealth_public::formerly())
        return stealth_public::symbol();
    if (former == stealth_secret::formerly())
        return stealth_secret::symbol();
    if (former == tx_sign::formerly())
        return tx_sign::symbol();
    if (former == validate_tx::formerly())
        return validate_tx::symbol();

    return "";

}

} // namespace explorer
} // namespace libbitcoin
