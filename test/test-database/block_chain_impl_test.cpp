#ifdef  BLOCK_CHAIN_IMPL_TESTS
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <metaverse/bitcoin.hpp>
#include <metaverse/explorer/define.hpp>
#include <metaverse/explorer/utility.hpp>
#include <metaverse/explorer/config/ec_private.hpp>
#include <metaverse/database/data_base.hpp>
#include <metaverse/bitcoin/chain/attachment/account/account.hpp>
#include <metaverse/bitcoin/chain/attachment/asset/asset.hpp>
#include <metaverse/bitcoin/chain/attachment/asset/asset_detail.hpp>
#include <metaverse/bitcoin/chain/attachment/asset/asset_transfer.hpp>
#include <metaverse/bitcoin/chain/attachment/etp/etp.hpp>
#include <metaverse/database/settings.hpp>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <string>
#include <metaverse/consensus/miner.hpp>
#include <metaverse/blockchain/block_chain_impl.hpp>
#include <metaverse/bitcoin/utility/threadpool.hpp>
#include <metaverse/bitcoin/config/base16.hpp>
#include <metaverse/bitcoin/utility/log.hpp>
using namespace libbitcoin::config;


using namespace libbitcoin::database;
using namespace libbitcoin::wallet;	
using namespace libbitcoin::explorer::config;	
using namespace libbitcoin::chain;	
using namespace libbitcoin;
using namespace libbitcoin::blockchain;

#define  LOG_BLOCK_CHAIN_IMPL_TEST "block_chaim_impl_test"
#define  NAME  "wdy"
#define  NAME2  "wdy2"
#define  PASSWD  "wdyoschina"
static std::shared_ptr<block_chain_impl> get_block_chain_impl()
{
	const boost::filesystem::path& directory = boost::filesystem::path("database"); // settings default value
	
	using namespace boost::system;
    boost::system::error_code ec;
    if (create_directories(directory, ec))
    {
        std::cout << format("Please wait while initializing %1% directory...") % directory;
		//const auto genesis = chain::block::genesis_mainnet();
        auto genesis = consensus::miner::create_genesis_block();
        const auto result = data_base::initialize(directory, *genesis);
    }
    //std::cout << format("ec result = %1%  %2%") % directory % ec.message();
    if (ec.value() == 0) // 0 == directory_exists
    {
        //std::cout << format("%1% exist") % directory;
    }
	
    const blockchain::settings chain; 
    const database::settings db;
	threadpool pool;
	
	return std::make_shared<block_chain_impl>(pool, chain, db);

}

BOOST_AUTO_TEST_SUITE(block_chain_impl_tests)

/*
	following function is executed!
	
	is_account_passwd_valid
	get_account
	get_hash
	store_account
 */
BOOST_AUTO_TEST_CASE(is_account_passwd_valid)
{	
	auto bc_impl = get_block_chain_impl();
	bc_impl->start();

	const std::string name = NAME;
	const std::string passwd = PASSWD;
	const std::string mnemonic = "mnemonic";

	auto acc = std::make_shared<account>();
	acc->set_name(name);
	acc->set_passwd(passwd);
	acc->set_mnemonic(mnemonic);
	bc_impl->store_account(acc);
	
	auto test_acc = bc_impl->is_account_passwd_valid(name, passwd);
	if(test_acc)
		log::info(LOG_BLOCK_CHAIN_IMPL_TEST) 
		<< "account=" << test_acc->to_string();
	bc_impl->stop();
    BOOST_REQUIRE(test_acc != nullptr);
}

/*
	following function is executed!
	
	store_account_address
	get_account_address
 */
BOOST_AUTO_TEST_CASE(get_account_address)
{	
	auto bc_impl = get_block_chain_impl();
	bc_impl->start();
	
	auto sh_addr = std::make_shared<account_address>(NAME, "xprv-key", "xpub-key" ,1000, 0, "alia", "address");
	bc_impl->store_account_address(sh_addr);
	sh_addr = std::make_shared<account_address>(NAME2, "xprv-key2", "xpub-key2" ,1000, 0, "alia2", "address2");
	bc_impl->store_account_address(sh_addr);

	auto ptr = bc_impl->get_account_address(NAME, "address");
	if(ptr)
		log::info(LOG_BLOCK_CHAIN_IMPL_TEST) <<ptr->to_string();
	
	ptr = bc_impl->get_account_address(NAME2, "address2");
	if(ptr)
		log::info(LOG_BLOCK_CHAIN_IMPL_TEST) <<ptr->to_string();
	
	bc_impl->stop();
    BOOST_REQUIRE(ptr != nullptr);
}
/* 
	store_account_address
	get_account_addresses
*/

BOOST_AUTO_TEST_CASE(get_account_addresses)
{	
	auto bc_impl = get_block_chain_impl();
	bc_impl->start();
	
	auto sh_addr = std::make_shared<account_address>(NAME, "xprv-key", "xpub-key" ,1000, 0, "alia", "address");
	bc_impl->store_account_address(sh_addr);
	sh_addr = std::make_shared<account_address>(NAME, "xprv-key2", "xpub-key2" ,1000, 0, "alia2", "address2");
	bc_impl->store_account_address(sh_addr);

	auto sh_vec = bc_impl->get_account_addresses(NAME);
	
	if(sh_vec->size()) {
		
	    const auto action = [&](account_address& elem)
	    {
	        log::info(LOG_BLOCK_CHAIN_IMPL_TEST) <<elem.to_string();
	    };
	    std::for_each(sh_vec->begin(), sh_vec->end(), action);
		
	}
	
	bc_impl->stop();
    BOOST_REQUIRE(sh_vec->size() != 0);
}

/*
	following function is executed!
	
	store_asset
	get_asset
 */
BOOST_AUTO_TEST_CASE(get_asset)
{	
	auto bc_impl = get_block_chain_impl();
	bc_impl->start();
	
	auto acc = std::make_shared<asset_detail>();
	acc->set_symbol("car1");
	acc->set_maximum_supply(1000);
	acc->set_asset_type(1);
	acc->set_issuer("wdy");
	acc->set_address("1viewxxx");
	acc->set_description("a car of wdy divide into 1000 shares!");
	
	bc_impl->store_asset(acc);
		
	auto test_acc = bc_impl->get_asset("car1");
	if(test_acc)
		log::info(LOG_BLOCK_CHAIN_IMPL_TEST) 
		<< "asset=" << test_acc->to_string();
	bc_impl->stop();
    BOOST_REQUIRE(test_acc != nullptr);
}

/*  test : just not issued asset
	not test : asset in blockchain
*/
BOOST_AUTO_TEST_CASE(get_account_asset)
{	
	auto bc_impl = get_block_chain_impl();
	bc_impl->start();

	// account name "wdy" own address "1viewfin1" own asset "car"
	// account name "wdy" own address "1viewfin2" own asset "house"
	// account name "eric" own address "1eric1" own asset "bike"
	// account name "eric" own address "1eric2" own asset "shares"
	
	const std::string passwd = PASSWD;
	const std::string mnemonic = "mnemonic";
	// create two accounts
	auto acc = std::make_shared<account>();
	acc->set_name("wdy");
	acc->set_passwd(passwd);
	acc->set_mnemonic(mnemonic);
	bc_impl->store_account(acc);

	acc = std::make_shared<account>();
	acc->set_name("eric");
	acc->set_passwd(passwd);
	acc->set_mnemonic(mnemonic);
	bc_impl->store_account(acc);

	// create two addresses for account "wdy"
	auto sh_addr = std::make_shared<account_address>("wdy", "xprv-key", "xpub-key" ,1000, 0, "alia", "1viewfin1");
	bc_impl->store_account_address(sh_addr);
	sh_addr = std::make_shared<account_address>("wdy", "xprv-key2", "xpub-key2" ,1000, 0, "alia2", "1viewfin2");
	bc_impl->store_account_address(sh_addr);

	// create two addresses for account "eric"
	sh_addr = std::make_shared<account_address>("eric", "xprv-key", "xpub-key" ,1000, 0, "alia", "1eric1");
	bc_impl->store_account_address(sh_addr);
	sh_addr = std::make_shared<account_address>("eric", "xprv-key2", "xpub-key2" ,1000, 0, "alia2", "1eric2");
	bc_impl->store_account_address(sh_addr);

	
	auto detail = std::make_shared<asset_detail>();
	detail->set_symbol("car");
	detail->set_maximum_supply(1000);
	detail->set_asset_type(1);
	detail->set_issuer("wdy");
	detail->set_address("car address");
	detail->set_description("a car of wdy divide into 1000 shares!");
	bc_impl->store_account_asset(detail);
	
	detail = std::make_shared<asset_detail>();
	detail->set_symbol("house");
	detail->set_maximum_supply(10000);
	detail->set_asset_type(1);
	detail->set_issuer("wdy");
	detail->set_address("house address");
	detail->set_description("a house of wdy divide into 10000 shares!");
	bc_impl->store_account_asset(detail);
	
	detail = std::make_shared<asset_detail>();
	detail->set_symbol("bike");
	detail->set_maximum_supply(1000);
	detail->set_asset_type(1);
	detail->set_issuer("eric");
	detail->set_address("bike address");
	detail->set_description("a bike of eric divide into 10 shares!");
	bc_impl->store_account_asset(detail);
	
	detail = std::make_shared<asset_detail>();
	detail->set_symbol("shares");
	detail->set_maximum_supply(10000);
	detail->set_asset_type(1);
	detail->set_issuer("eric");
	detail->set_address("shares address");
	detail->set_description("a shares of eric divide into 10000 shares!");
	bc_impl->store_account_asset(detail);
	
	auto sh_vec = bc_impl->get_account_asset("wdy", "car");
	log::info(LOG_BLOCK_CHAIN_IMPL_TEST) << "wdy car = " << sh_vec->size();
	if(sh_vec->size()) {
		
		const auto action = [&](business_address_asset& elem)
		{
			log::info(LOG_BLOCK_CHAIN_IMPL_TEST) <<elem.to_string();
		};
		std::for_each(sh_vec->begin(), sh_vec->end(), action);
	}

	sh_vec = bc_impl->get_account_asset("wdy", "house");
	log::info(LOG_BLOCK_CHAIN_IMPL_TEST) << "wdy house = " << sh_vec->size();
	if(sh_vec->size()) {
		
		const auto action = [&](business_address_asset& elem)
		{
			log::info(LOG_BLOCK_CHAIN_IMPL_TEST) <<elem.to_string();
		};
		std::for_each(sh_vec->begin(), sh_vec->end(), action);
	}
	
	sh_vec = bc_impl->get_account_asset("eric", "bike");
	log::info(LOG_BLOCK_CHAIN_IMPL_TEST) << "eric bike = " << sh_vec->size();
	if(sh_vec->size()) {
		
		const auto action = [&](business_address_asset& elem)
		{
			log::info(LOG_BLOCK_CHAIN_IMPL_TEST) <<elem.to_string();
		};
		std::for_each(sh_vec->begin(), sh_vec->end(), action);
	}
	
	sh_vec = bc_impl->get_account_asset("eric", "shares");
	log::info(LOG_BLOCK_CHAIN_IMPL_TEST) << "eric shares = " << sh_vec->size();
	if(sh_vec->size()) {
		
		const auto action = [&](business_address_asset& elem)
		{
			log::info(LOG_BLOCK_CHAIN_IMPL_TEST) <<elem.to_string();
		};
		std::for_each(sh_vec->begin(), sh_vec->end(), action);
	}
	
	sh_vec = bc_impl->get_account_assets("wdy");
	log::info(LOG_BLOCK_CHAIN_IMPL_TEST) << "wdy = " << sh_vec->size();
	if(sh_vec->size()) {
		
		const auto action = [&](business_address_asset& elem)
		{
			log::info(LOG_BLOCK_CHAIN_IMPL_TEST) <<elem.to_string();
		};
		std::for_each(sh_vec->begin(), sh_vec->end(), action);
	}
	
	sh_vec = bc_impl->get_account_assets("eric");
	log::info(LOG_BLOCK_CHAIN_IMPL_TEST) << "eric = " << sh_vec->size();
	if(sh_vec->size()) {
		
		const auto action = [&](business_address_asset& elem)
		{
			log::info(LOG_BLOCK_CHAIN_IMPL_TEST) <<elem.to_string();
		};
		std::for_each(sh_vec->begin(), sh_vec->end(), action);
	}
	
	bc_impl->stop();
    BOOST_REQUIRE(sh_vec->size() != 0);
}

BOOST_AUTO_TEST_SUITE_END()
#endif

