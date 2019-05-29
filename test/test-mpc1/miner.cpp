
#include <metaverse/consensus/miner.hpp>
#include <metaverse/macros_define.hpp>

#include <algorithm>
#include <functional>
#include <system_error>
#include <chrono>
#include <ctime>
#include <metaverse/consensus/miner/MinerAux.h>
#include <metaverse/consensus/libdevcore/BasicType.h>
#include <metaverse/consensus/witness.hpp>
#include <metaverse/bitcoin/chain/script/operation.hpp>
#include <metaverse/bitcoin/config/hash160.hpp>
#include <metaverse/bitcoin/wallet/ec_public.hpp>
#include <metaverse/bitcoin/utility/random.hpp>
#include <metaverse/bitcoin/constants.hpp>
#include <metaverse/blockchain/validate_block.hpp>
#include <metaverse/blockchain/validate_transaction.hpp>
#include <metaverse/blockchain/block_chain.hpp>
#include <metaverse/blockchain/block_chain_impl.hpp>
#include <metaverse/node/p2p_node.hpp>
#include <metaverse/macros_define.hpp>

#include <metaverse/bitcoin/constants.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(mpc1_test)

using namespace bc::consensus;
using namespace bc;


BOOST_AUTO_TEST_CASE(calculate_block_subsidy_pow_test)
{
    BOOST_CHECK(miner::calculate_block_subsidy_pow(499999, false) == 300000000);
    BOOST_CHECK(miner::calculate_block_subsidy_pow(1499999, false) == 270750000);
    BOOST_CHECK(miner::calculate_block_subsidy_pow(1900000, false) == 257212499);
    BOOST_CHECK(miner::calculate_block_subsidy_pow(1999999, false) == 257212499);
    BOOST_CHECK(miner::calculate_block_subsidy_pow(2000000, false) == 244351874);
    BOOST_CHECK(miner::calculate_block_subsidy_pow(2499999, false) == 244351874);
    BOOST_CHECK(miner::calculate_block_subsidy_pow(2500000, false) == 193445234);
    BOOST_CHECK(miner::calculate_block_subsidy_pow(2999999, false) == 193445234);
    BOOST_CHECK(miner::calculate_block_subsidy_pow(3000000, false) == 183772972);

    bc::pos_enabled_height = 990000; //testnet
    BOOST_CHECK(miner::calculate_block_subsidy_pow(989999, true) == 113206080);
    BOOST_CHECK(miner::calculate_block_subsidy_pow(990000, true) == 94338400);
    BOOST_CHECK(miner::calculate_block_subsidy_pow(999999, true) == 94338400);
    BOOST_CHECK(miner::calculate_block_subsidy_pow(1000000, true) == 89621480);
    BOOST_CHECK(miner::calculate_block_subsidy_pow(1050000, true) == 85140406);
    BOOST_CHECK(miner::calculate_block_subsidy_pow(1100000, true) == 80883386);

}

BOOST_AUTO_TEST_CASE(calculate_block_subsidy_pos_test)
{
    BOOST_CHECK(miner::calculate_block_subsidy_pos(1924000, false) == 25721249);
    BOOST_CHECK(miner::calculate_block_subsidy_pos(1999999, false) == 25721249);
    BOOST_CHECK(miner::calculate_block_subsidy_pos(2000000, false) == 24435187);
    BOOST_CHECK(miner::calculate_block_subsidy_pos(2499999, false) == 24435187);
    BOOST_CHECK(miner::calculate_block_subsidy_pos(2500000, false) == 23213428);
    BOOST_CHECK(miner::calculate_block_subsidy_pos(2999999, false) == 23213428);
    BOOST_CHECK(miner::calculate_block_subsidy_pos(3000000, false) == 22052756);

    bc::pos_enabled_height = 990000; //testnet
    BOOST_CHECK(miner::calculate_block_subsidy_pos(989999, true) == 11320608);
    BOOST_CHECK(miner::calculate_block_subsidy_pos(990000, true) == 11320608);
    BOOST_CHECK(miner::calculate_block_subsidy_pos(999999, true) == 11320608);
    BOOST_CHECK(miner::calculate_block_subsidy_pos(1000000, true) == 10754577);
    BOOST_CHECK(miner::calculate_block_subsidy_pos(1050000, true) ==10216848);
    BOOST_CHECK(miner::calculate_block_subsidy_pos(1100000, true) == 9706006);
}

BOOST_AUTO_TEST_SUITE_END()
