/*
 * threadpool.cpp
 *
 *  Created on: Nov 29, 2016
 *      Author: jiang
 */


#include <boost/test/unit_test.hpp>
#include <bitcoin/bitcoin/utility/threadpool.hpp>
#include <bitcoin/bitcoin/utility/deadline.hpp>
#include <bitcoin/bitcoin/utility/asio.hpp>
#include <memory>

BOOST_AUTO_TEST_SUITE(suit_threadpool)

BOOST_AUTO_TEST_CASE(case_threadpool)
{
	try
	{
		libbitcoin::threadpool pool_{4};
		pool_.spawn(4, libbitcoin::thread_priority::low);
		auto line = std::make_shared<libbitcoin::deadline>(pool_, libbitcoin::asio::seconds(1));

		line->start([&pool_](const libbitcoin::code& ec){
			std::cout << "hello world!!!" << std::endl;
			pool_.shutdown();
		});

		pool_.join();
	}
	catch(const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
}


BOOST_AUTO_TEST_SUITE_END()
