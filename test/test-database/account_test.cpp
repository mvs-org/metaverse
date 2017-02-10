#ifdef  ACCOUNT_TESTS
#include <iostream>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/explorer/define.hpp>
#include <bitcoin/explorer/utility.hpp>
#include <bitcoin/explorer/config/ec_private.hpp>
#include <string>
#include <boost/test/unit_test.hpp>

using namespace libbitcoin::explorer;
using namespace libbitcoin::wallet;	
using namespace libbitcoin::explorer::config;	
using namespace libbitcoin::config;	
using namespace libbitcoin;

const std::string search = "1VIEW";

#define  ACCOUNT_TEST  "account_test"
class  account 
{
public:
	void generate_key_pair();
	void generate_hd_key_pair();
	// 
	std::string generate_bc_address();
	bool match_found(const std::string& address);
};
void account::generate_key_pair()
{
    const auto seed = new_seed(16*8);
    log::info(ACCOUNT_TEST) << base16(seed)  ;
	
	
    ec_secret secret(new_key(seed));
	#if 0
    if (secret == null_hash)
    {
        error << BX_EC_NEW_INVALID_KEY  ;
        return console_result::failure;
    }
	#endif

    // We don't use bc::ec_private serialization (WIF) here.
    log::info(ACCOUNT_TEST) << libbitcoin::explorer::config::ec_private(secret)  ;

	
    ec_compressed point;
    secret_to_public(point, secret);

    // Serialize to the original compression state.
    log::info(ACCOUNT_TEST) << ec_public(point, true)  ;

	
    log::info(ACCOUNT_TEST) << payment_address(point)  ;
	
	payment_address addr = payment_address("1viEwgKdzDUxoMJr7soChGZX45g5KD6nK");
	log::info(ACCOUNT_TEST) << "version=" << addr.version();
	//log::info(ACCOUNT_TEST) << "hash=" << addr.hash().data();
	log::info(ACCOUNT_TEST) << "encoded=" << addr.encoded();
	
}

std::string account::generate_bc_address()
{
    const auto seed = new_seed(16*8);
    log::info(ACCOUNT_TEST) << base16(seed)  ;
	
    ec_secret secret(new_key(seed));
    // We don't use bc::ec_private serialization (WIF) here.
    log::info(ACCOUNT_TEST) << libbitcoin::explorer::config::ec_private(secret)  ;

	
    ec_compressed point;
    secret_to_public(point, secret);

    // Serialize to the original compression state.
    log::info(ACCOUNT_TEST) << ec_public(point, true)  ;
	
    log::info(ACCOUNT_TEST) << payment_address(point)  ;
	return payment_address(point).encoded();
}

bool account::match_found(const std::string& address)
{	
	auto addr_it = address.begin();	
	// Loop through the search std::string comparing it to the lower case	
	// character of the supplied address.	
	for (auto it = search.begin(); it != search.end(); ++it, ++addr_it)	
		if (std::tolower(*it) != std::tolower(*addr_it))	
			return false;	// Reached end of search std::string, so address matches.	
	return true;

}


#include <bitcoin/bitcoin/wallet/mnemonic.hpp>
#include <bitcoin/bitcoin/wallet/hd_private.hpp>
void account::generate_hd_key_pair()
{
	// seed
    const auto seed = new_seed(16*8);
    log::info(ACCOUNT_TEST) << base16(seed);
	
	// mnemonic-new
	const auto entropy_size = seed.size();
	
	if ((entropy_size % libbitcoin::wallet::mnemonic_seed_multiple) != 0)
	{
		log::error(ACCOUNT_TEST) << "seed size error";
		return ;
	}
	//const auto dictionary = libbitcoin::wallet::language::en;
	const auto dictionary = libbitcoin::wallet::language::zh_Hans;
	const auto words = create_mnemonic(seed, dictionary);
	log::info(ACCOUNT_TEST) << join(words);

	// mnemonic-to-seed
	const auto mn_seed = decode_mnemonic(words);
	log::info(ACCOUNT_TEST) << base16(mn_seed) ;

	// hd-new
    // We require the private version, but public is unused here.
    //const auto prefixes = libbitcoin::wallet::hd_private::to_prefixes(version, 0);
    //const auto prefixes = libbitcoin::wallet::hd_private::mainnet;
    const libbitcoin::wallet::hd_private private_key(to_chunk(mn_seed));

    if (!private_key)
    {
        log::error(ACCOUNT_TEST) << "generate hd private key error!";
        return ;
    }

    log::info(ACCOUNT_TEST) << private_key ;
    log::info(ACCOUNT_TEST) << private_key.to_public() ;

}
BOOST_AUTO_TEST_SUITE(account_tests )

BOOST_AUTO_TEST_CASE(get_private_public_key)
{
	using namespace libbitcoin;

	using namespace libbitcoin::explorer;
	
     
	account acc;
    acc.generate_key_pair();
    BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE(get_hd_private_public_key)
{
	using namespace libbitcoin;

	using namespace libbitcoin::explorer;
	
     
	account acc;
    acc.generate_hd_key_pair();
    BOOST_REQUIRE(true);
}
#if 0
BOOST_AUTO_TEST_CASE(get_beauty_key)
{
	using namespace libbitcoin;

	using namespace libbitcoin::explorer;
     
	account acc;
	
	while(true)
	{
		const std::string address = acc.generate_bc_address();
		// Loop through the search std::string comparing it to the lower case	
		// character of the supplied address.	
		if (acc.match_found(address))		
		{			
			// Success!			
			log::info(ACCOUNT_TEST) << "Found address! " << address  ;			
			break;	
		}
		
	}
    BOOST_REQUIRE(true);
}
#endif

BOOST_AUTO_TEST_SUITE_END()
#endif
