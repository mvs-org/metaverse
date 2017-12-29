/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse.
 *
 * metaverse is free software: you can redistribute it and/or modify
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
#include <metaverse/blockchain/settings.hpp>

#include <boost/filesystem.hpp>
#include <metaverse/consensus/miner.hpp>
#include <metaverse/bitcoin/constants.hpp>

namespace libbitcoin {
namespace blockchain {

using namespace boost::filesystem;

settings::settings()
  : block_pool_capacity(5000),
    transaction_pool_capacity(4096),
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
            checkpoints.push_back({ "b81848ef9ae86e84c3da26564bc6ab3a79efc628239d11471ab5cd25c0684c2d", 0 });
            // fixme. header sync first has some problem.
            //checkpoints.push_back({ "250a083ddd62ea1d0907e29ff8d64e42c451f93560196f3f693fdc1bc6b84d61", 10000 });
            //checkpoints.push_back({ "e989e4b2d60ae2f8fbaa1cdb69d05afa63e7f1f99cf715589a93e4877c92fa8b", 100000 });
            //checkpoints.push_back({ "58986f8f9848d32aa1a210f3890e82312326657b6b84d2aa4bf00b41403dc191", 200000 });
            //checkpoints.push_back({ "b9ec93b181e5ca3825df23c8100188a503b98d6e7240c7b21cedc980304967ea", 300000 });
            //checkpoints.push_back({ "843411e1194c11cc958abd923498e1a7488ba9b0bccf1a3a5960f3bc213645ce", 400000 });
            //checkpoints.push_back({ "2be656ee2c84684faaefd4e9f21f157d1dcb6ad9d25bf18d037cea37d23437ca", 500000 });
            //checkpoints.push_back({ "265e051b24034d3bb51e99099a98d1d103c703cdac22a0d52e816aeb9cb807b9", 600000 });
            //checkpoints.push_back({ "bd512ef95e5c6c99bf03112be7ac7fc0a6ef1113678dd583a18778ca683908f9", 700000 });
            //checkpoints.push_back({ "9a0efd7b41cfc1cbeb1bfbd2ab3cb7989314611608cc4236b80a540444fbfb36", 800000 });

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
            libbitcoin::consensus::lock_heights = {10, 20, 30, 40, 50};
            libbitcoin::coinbase_maturity = 1;
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
