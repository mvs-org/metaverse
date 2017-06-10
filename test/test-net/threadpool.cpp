/*
 * threadpool.cpp
 *
 *  Created on: Nov 29, 2016
 *      Author: jiang
 */


#include <boost/test/unit_test.hpp>
#include <metaverse/bitcoin/utility/threadpool.hpp>
#include <metaverse/bitcoin/utility/deadline.hpp>
#include <metaverse/bitcoin/utility/asio.hpp>
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


BOOST_AUTO_TEST_CASE(case_interupt)
{
	boost::thread th{[](){
		while(true)
		{
			std::cout << "aaa" << std::endl;
			boost::this_thread::interruption_point();
			boost::this_thread::sleep(boost::posix_time::milliseconds{100});
		}
	}};
	boost::this_thread::sleep(boost::posix_time::seconds{1});
	th.interrupt();
	boost::this_thread::sleep(boost::posix_time::seconds{10});
}


BOOST_AUTO_TEST_SUITE_END()
