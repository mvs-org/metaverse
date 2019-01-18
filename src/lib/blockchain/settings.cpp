/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
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
#include <metaverse/macros_define.hpp>

namespace libbitcoin {
namespace blockchain {

using namespace boost::filesystem;

settings::settings()
  : block_pool_capacity(5000),
    transaction_pool_capacity(4096),
    transaction_pool_consistency(false),
    use_testnet_rules(false),
    collect_split_stake(true),
    checkpoints(),
    basic_checkpoints()
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
            // FIXEME.p2p-node.cpp, This is for header-first sync, attach_header_sync_session
            // header sync first has some problem.
            basic_checkpoints.reserve(1);
            basic_checkpoints.push_back({ "b81848ef9ae86e84c3da26564bc6ab3a79efc628239d11471ab5cd25c0684c2d", 0 });

            // validate_block, This is for each single node validate
            checkpoints.reserve(7);
            checkpoints.push_back({ "b81848ef9ae86e84c3da26564bc6ab3a79efc628239d11471ab5cd25c0684c2d", 0 });
            checkpoints.push_back({ "d4f84eb9acb52cbfb003bd4fb88f4b5cf1f6328f193097d42b9403d7030abb8e", 3800 });
            checkpoints.push_back({ "e989e4b2d60ae2f8fbaa1cdb69d05afa63e7f1f99cf715589a93e4877c92fa8b", 100000 });
            checkpoints.push_back({ "d120edf7e14d8f426b7390e94ae7bfd746f9fc247b28e336db052295177ac9d3", 410000 }); //0.6.9 fix
            checkpoints.push_back({ "9a0efd7b41cfc1cbeb1bfbd2ab3cb7989314611608cc4236b80a540444fbfb36", 800000 });
            checkpoints.push_back({ "ed11a074ce80cbf82b5724bea0d74319dc6f180198fa1bbfb562bcbd50089e63", 1030000 }); //future time attack
            checkpoints.push_back({ "a18a5aa5270835dd720a561c20230435e0508e8339ee30998da6dd79eee83b29", 1270000 }); //super nova

            bc::wallet::ec_private::mainnet_p2kh = 0x32;
            bc::wallet::ec_public::mainnet_p2kh = 0x32;
            bc::wallet::payment_address::mainnet_p2kh = 0x32;
            break;
        }

        case bc::settings::testnet:
        {
            use_testnet_rules = true;

            basic_checkpoints.reserve(1);
            basic_checkpoints.push_back({ "c359a1cc3dfb8b97111c3e602f1f6de31306926f9ec779cb9ea002edbee91741", 0 });

            checkpoints.reserve(1);
            checkpoints.push_back({ "c359a1cc3dfb8b97111c3e602f1f6de31306926f9ec779cb9ea002edbee91741", 0 });

            bc::wallet::ec_private::mainnet_p2kh = 0x7f;
            bc::wallet::ec_public::mainnet_p2kh = 0x7f;
            bc::wallet::payment_address::mainnet_p2kh = 0x7f;

            libbitcoin::consensus::lock_heights = {10, 20, 30, 40, 50};
            libbitcoin::coinbase_maturity       = 1;

#ifndef PRIVATE_CHAIN
            libbitcoin::pos_enabled_height      = 990000;
#endif
            break;
        }

        default:
        case bc::settings::none:
        {
        }
    }

#ifdef PRIVATE_CHAIN
    {
        basic_checkpoints.clear();
        basic_checkpoints.push_back({ "b694bd9fae0fec221e24bde774d5414df994ed536909c4ffca49d55d0f14f40f", 0 });

        checkpoints.clear();
        checkpoints.push_back({ "b694bd9fae0fec221e24bde774d5414df994ed536909c4ffca49d55d0f14f40f", 0 });
    }
#endif

}

} // namespace blockchain
} // namespace libbitcoin
