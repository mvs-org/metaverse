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
#include <metaverse/explorer/commands/stealth-public.hpp>

#include <iostream>
#include <metaverse/bitcoin.hpp>
#include <metaverse/explorer/define.hpp>


namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::wallet;

// This is nearly the same as ec-add.
console_result stealth_public::invoke(std::ostream& output,
    std::ostream& error)
{
    // Bound parameters.
    const auto& spend_pubkey = get_spend_pubkey_argument();
    const auto& shared_secret = get_shared_secret_argument();

    ec_compressed sum(spend_pubkey);
    if (!bc::ec_add(sum, shared_secret))
    {
        error << BX_STEALTH_PUBLIC_OUT_OF_RANGE << std::flush;
        return console_result::failure;
    }

    output << ec_public(sum) << std::flush;
    return console_result::okay;
}

} //namespace commands
} //namespace explorer
} //namespace libbitcoin
