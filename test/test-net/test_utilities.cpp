/*
 * test_utilities.cpp
 *
 *  Created on: Feb 23, 2017
 *      Author: jiang
 */

#include <boost/test/unit_test.hpp>
#include <metaverse/bitcoin.hpp>

BOOST_AUTO_TEST_SUITE(test_utitlities)



BOOST_AUTO_TEST_CASE(test_deadline){
	libbitcoin::threadpool threadpool{4};
	auto d = libbitcoin::asio::duration{5000};
	libbitcoin::deadline::ptr deadline = std::make_shared<libbitcoin::deadline>(threadpool, d);
	auto handle = [](const libbitcoin::code& ec){
		std::cout << "hello" << std::endl;
	};
//	deadline->start([](const libbitcoin::code& ec){
//		std::cout << "hello1" << std::endl;
//	});
	deadline->start([](const libbitcoin::code& ec){
		std::cout << "hello2" << std::endl;
	});
	threadpool.join();
}




BOOST_AUTO_TEST_SUITE_END()


