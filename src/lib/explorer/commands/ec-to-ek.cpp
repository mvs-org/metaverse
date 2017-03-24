/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
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
#include <metaverse/lib/explorer/commands/ec-to-ek.hpp>

#include <iostream>
#include <metaverse/lib/bitcoin.hpp>
#include <metaverse/lib/explorer/define.hpp>


namespace libbitcoin {
namespace explorer {
namespace commands {
using namespace bc::wallet;

console_result ec_to_ek::invoke(std::ostream& output, std::ostream& error)
{
#ifdef WITH_ICU
    const auto uncompressed = get_uncompressed_option();
    const auto version = get_version_option();
    const auto& passphrase = get_passphrase_argument();
    const auto& secret = get_ec_private_key_argument();

    encrypted_private point;
    encrypt(point, secret, passphrase, version, !uncompressed);

    output << ek_private(point) << std::flush;
    return console_result::okay;
#else
    error << BX_EC_TO_EK_REQUIRES_ICU << std::flush;
    return console_result::failure;
#endif
}

} //namespace commands 
} //namespace explorer 
} //namespace libbitcoin 
