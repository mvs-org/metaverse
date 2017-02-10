#ifdef  DATABASE_TESTS
#include <iostream>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/explorer/define.hpp>
#include <bitcoin/explorer/utility.hpp>
#include <bitcoin/explorer/config/ec_private.hpp>
#include <bitcoin/database/data_base.hpp>
#include <bitcoin/bitcoin/chain/attachment/account/account.hpp>
#include <bitcoin/bitcoin/chain/attachment/asset/asset.hpp>
#include <bitcoin/bitcoin/chain/attachment/asset/asset_detail.hpp>
#include <bitcoin/bitcoin/chain/attachment/asset/asset_transfer.hpp>
#include <bitcoin/bitcoin/chain/attachment/etp/etp.hpp>
#include <bitcoin/database/settings.hpp>
#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>
#include <string>
#include <boost/test/unit_test.hpp>

#include <bitcoin/bitcoin/config/base16.hpp>
using namespace libbitcoin::config;

#include <bitcoin/consensus/miner.hpp>
using namespace libbitcoin::consensus;

using namespace libbitcoin::database;
using namespace libbitcoin::wallet;	
using namespace libbitcoin::explorer::config;	
using namespace libbitcoin::chain;	
using namespace libbitcoin;

#define  LOG_DATABASE_TEST "database_test"
static std::shared_ptr<data_base> get_database_instance()
{
	const boost::filesystem::path& directory = boost::filesystem::path("database"); 
	
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
    database::settings db_settings;
	
	return std::make_shared<data_base>(db_settings);

}

static attachment get_attachment_from_file(const std::string& filename)
{
	data_chunk vec;
	{
		std::filebuf fb;
		if (fb.open (filename,std::ios::in))
		{
			std::istream is(&fb);
			while (is) {
				vec.push_back(uint8_t(is.get()));
			}
			fb.close();
			//std::cout<<vec.size();
		}
	}
	//for_each(vec.begin(), vec.end(), out_func);
	//std::cout<<std::endl;
	return attachment::factory_from_data(vec);
}

//BOOST_FIXTURE_TEST_SUITE(data_base_tests)
BOOST_AUTO_TEST_SUITE(data_base_tests)

BOOST_AUTO_TEST_CASE(push_attachemnt_etp)
{
	auto sh_db = get_database_instance();
	sh_db->start();
	
	auto attach = get_attachment_from_file("attachment-with-etp.data");
	payment_address addr = payment_address("1viEwgKdzDUxoMJr7soChGZX45g5KD6nK");
	output_point outpoint{hash_digest(), 0}; 
	log::info(LOG_DATABASE_TEST) << attach.to_string();
	sh_db->push_attachemnt(attach, addr, outpoint, 1000, 10);
	
	// business_address_asset::list
	//auto address_str = addr.encoded();
	//data_chunk data(address_str.begin(), address_str.end());
	//short_hash hash = ripemd160_hash(data);
	//log::info(LOG_DATABASE_TEST) << "address_str = " << address_str;
	auto sh_vec = sh_db->address_assets.get_assets(addr.encoded(), 0);
	log::info(LOG_DATABASE_TEST) << "size = " << sh_vec.size();
	if(sh_vec.size()) {
		
		const auto action = [&](business_address_asset& elem)
		{
			log::info(LOG_DATABASE_TEST) <<elem.to_string();
		};
		std::for_each(sh_vec.begin(), sh_vec.end(), action);
	}
    sh_db->stop();
    BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE(push_attachemnt_asset_detail)
{
	auto sh_db = get_database_instance();
	sh_db->start();
	
	auto attach = get_attachment_from_file("attachment-with-asset-detail.data");
	payment_address addr = payment_address("1viEwgKdzDUxoMJr7soChGZX45g5KD6nK");
	output_point outpoint{hash_digest(), 0}; 
	log::info(LOG_DATABASE_TEST) << attach.to_string();
	sh_db->push_attachemnt(attach, addr, outpoint, 1000, 10);
	
	// business_address_asset::list 
	auto sh_vec = sh_db->address_assets.get_assets("1viEwgKdzDUxoMJr7soChGZX45g5KD6nK", 0);
	log::info(LOG_DATABASE_TEST) << "size = " << sh_vec.size();
	if(sh_vec.size()) {
		
		const auto action = [&](business_address_asset& elem)
		{
			log::info(LOG_DATABASE_TEST) <<elem.to_string();
		};
		std::for_each(sh_vec.begin(), sh_vec.end(), action);
	}
    sh_db->stop();
    BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE(push_attachemnt_asset_transfer)
{
	auto sh_db = get_database_instance();
	sh_db->start();
	
	auto attach = get_attachment_from_file("attachment-with-asset-transfer.data");
	payment_address addr = payment_address("1viEwgKdzDUxoMJr7soChGZX45g5KD6nK");
	output_point outpoint{null_hash, 0}; 
	log::info(LOG_DATABASE_TEST) << attach.to_string();
	sh_db->push_attachemnt(attach, addr, outpoint, 1000, 10);
	
	// business_address_asset::list 
	auto sh_vec = sh_db->address_assets.get_assets("1viEwgKdzDUxoMJr7soChGZX45g5KD6nK", 0);
	log::info(LOG_DATABASE_TEST) << "size = " << sh_vec.size();
	if(sh_vec.size()) {
		
		const auto action = [&](business_address_asset& elem)
		{
			log::info(LOG_DATABASE_TEST) <<elem.to_string();
		};
		std::for_each(sh_vec.begin(), sh_vec.end(), action);
	}
    sh_db->stop();
    BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_SUITE_END()
#endif

