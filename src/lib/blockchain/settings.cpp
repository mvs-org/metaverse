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
#include <bitcoin/consensus/miner.hpp>

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
            bc::wallet::ec_private::mainnet_p2kh = 0x32;
            bc::wallet::ec_public::mainnet_p2kh = 0x32;
            bc::wallet::payment_address::mainnet_p2kh = 0x32;
            break;
        }

        case bc::settings::testnet:
        {
            use_testnet_rules = true;

            checkpoints.reserve(1);
            checkpoints.push_back({ "b1076144f919c8efaf55e5ec267daa6d5dc0a12609c9c6fddf8157270ae6e7ca", 0 });

            bc::wallet::ec_private::mainnet_p2kh = 0x7f;
            bc::wallet::ec_public::mainnet_p2kh = 0x7f;
            bc::wallet::payment_address::mainnet_p2kh = 0x7f;

            libbitcoin::consensus::bucket_size = 50000;
            libbitcoin::consensus::lock_heights = {10, 20, 30};
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
