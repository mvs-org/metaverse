/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <bitcoin/blockchain/settings.hpp>

#include <boost/filesystem.hpp>

namespace libbitcoin {
namespace blockchain {

using namespace boost::filesystem;

settings::settings()
  : block_pool_capacity(50),
    transaction_pool_capacity(1000),
    transaction_pool_consistency(false),
    use_testnet_rules(false)
{
}

// Use push_back due to initializer_list bug:
// stackoverflow.com/a/20168627/1172329
settings::settings(bc::settings context)
  : settings()
{
    switch (context)
    {
        case bc::settings::mainnet:
        {
            checkpoints.reserve(1);
            checkpoints.push_back({ "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f", 0 });
            break;
        }

        case bc::settings::testnet:
        {
            use_testnet_rules = true;

            checkpoints.reserve(1);
            checkpoints.push_back({ "000000000933ea01ad0ee984209779baaec3ced90fa3f408719526f8d77f4943", 0 });
            break;
        }

        default:
        case bc::settings::none:
        {
        }
    }
}

} // namespace blockchain
} // namespace libbitcoin
