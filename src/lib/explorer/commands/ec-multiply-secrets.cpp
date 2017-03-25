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
#include <metaverse/explorer/commands/ec-multiply-secrets.hpp>

#include <iostream>
#include <metaverse/bitcoin.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/config/ec_private.hpp>



namespace libbitcoin {
namespace explorer {
namespace commands {

console_result ec_multiply_secrets::invoke(std::ostream& output, std::ostream& error)
{
    // Bound parameters.
    const auto& secrets = get_secrets_argument();

    ec_secret product(null_hash);
    for (auto const& secret: secrets)
    {
        // Initialize product on first pass.
        if (product == null_hash)
        {
            product = secret;
            continue;
        }

        // Elliptic curve function (INTEGER * INTEGER) % curve-order.
        if (!bc::ec_multiply(product, secret))
        {
            error << BX_EC_MULITPLY_SECRETS_OUT_OF_RANGE << std::flush;
            return console_result::failure;
        }
    }

    // We don't use bc::ec_private serialization (WIF) here.
    output << config::ec_private(product) << std::flush;
    return console_result::okay;
}

} //namespace commands 
} //namespace explorer 
} //namespace libbitcoin 
