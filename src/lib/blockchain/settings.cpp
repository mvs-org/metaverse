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
            checkpoints.reserve(22);
            checkpoints.push_back({ "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f", 0 });
            checkpoints.push_back({ "0000000069e244f73d78e8fd29ba2fd2ed618bd6fa2ee92559f542fdb26e7c1d", 11111 });
            checkpoints.push_back({ "000000002dd5588a74784eaa7ab0507a18ad16a236e7b1ce69f00d7ddfb5d0a6", 33333 });
            checkpoints.push_back({ "00000000001e1b4903550a0b96e9a9405c8a95f387162e4944e8d9fbe501cd6a", 68555 });
            checkpoints.push_back({ "00000000006a49b14bcf27462068f1264c961f11fa2e0eddd2be0791e1d4124a", 70567 });
            checkpoints.push_back({ "0000000000573993a3c9e41ce34471c079dcf5f52a0e824a81e7f953b8661a20", 74000 });
            checkpoints.push_back({ "00000000000291ce28027faea320c8d2b054b2e0fe44a773f3eefb151d6bdc97", 105000 });
            checkpoints.push_back({ "000000000000774a7f8a7a12dc906ddb9e17e75d684f15e00f8767f9e8f36553", 118000 });
            checkpoints.push_back({ "00000000000005b12ffd4cd315cd34ffd4a594f430ac814c91184a0d42d2b0fe", 134444 });
            checkpoints.push_back({ "000000000000033b512028abb90e1626d8b346fd0ed598ac0a3c371138dce2bd", 140700 });
            checkpoints.push_back({ "000000000000099e61ea72015e79632f216fe6cb33d7899acb35b75c8303b763", 168000 });
            checkpoints.push_back({ "000000000000059f452a5f7340de6682a977387c17010ff6e6c3bd83ca8b1317", 193000 });
            checkpoints.push_back({ "000000000000048b95347e83192f69cf0366076336c639f9b7228e9ba171342e", 210000 });
            checkpoints.push_back({ "00000000000001b4f4b433e81ee46494af945cf96014816a4e2370f11b23df4e", 216116 });
            checkpoints.push_back({ "00000000000001c108384350f74090433e7fcf79a606b8e797f065b130575932", 225430 });
            checkpoints.push_back({ "000000000000003887df1f29024b06fc2200b55f8af8f35453d7be294df2d214", 250000 });
            checkpoints.push_back({ "0000000000000001ae8c72a0b0c301f67e3afca10e819efa9041e458e9bd7e40", 279000 });
            checkpoints.push_back({ "00000000000000004d9b4ef50f0f9d686fd69db2e03af35a100370c64632a983", 295000 });
            checkpoints.push_back({ "000000000000000017a4b176294583519076f06cd2b5e4ef139dada8d44838d8", 330791 });
            checkpoints.push_back({ "000000000000000017522241d7afd686bb2315930fc1121861c9abf52e8c37f1", 337459 });
            checkpoints.push_back({ "0000000000000000086672a8c97ad666f89cf04736951791150015419810d586", 368991 });
            checkpoints.push_back({ "000000000000000004ec466ce4732fe6f1ed1cddc2ed4b328fff5224276e3f6f", 400000 });
            break;
        }

        case bc::settings::testnet:
        {
            use_testnet_rules = true;

            checkpoints.reserve(7);
            checkpoints.push_back({ "000000000933ea01ad0ee984209779baaec3ced90fa3f408719526f8d77f4943", 0 });
            checkpoints.push_back({ "00000000009e2958c15ff9290d571bf9459e93b19765c6801ddeccadbb160a1e", 100000 });
            checkpoints.push_back({ "0000000000287bffd321963ef05feab753ebe274e1d78b2fd4e2bfe9ad3aa6f2", 200000 });
            checkpoints.push_back({ "000000000000226f7618566e70a2b5e020e29579b46743f05348427239bf41a1", 300000 });
            checkpoints.push_back({ "000000000598cbbb1e79057b79eef828c495d4fc31050e6b179c57d07d00367c", 400000 });
            checkpoints.push_back({ "000000000001a7c0aaa2630fbb2c0e476aafffc60f82177375b2aaa22209f606", 500000 });
            checkpoints.push_back({ "000000000000624f06c69d3a9fe8d25e0a9030569128d63ad1b704bbb3059a16", 600000 });
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
