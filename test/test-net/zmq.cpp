/*
 * zmq.cpp
 *
 *  Created on: Nov 28, 2016
 *      Author: jiang
 */

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>
#include <zmq.h>


BOOST_AUTO_TEST_SUITE(test_zmq)

BOOST_AUTO_TEST_CASE(case_pub_sub)
{
	try
	{
		auto ctx = zmq_ctx_new();
		for(auto i=0;i<2048;++i)
		{
			auto pub = zmq_socket(ctx, ZMQ_PUB);
			if(pub == nullptr)
			{
				std::cout << "connect failed" << std::endl;
			}
			auto res = zmq_connect(pub, "inproc://test");
			if(res == -1)
			{
				std::cout << "connect failed" << std::endl;
			}
			res = zmq_close(pub);
			if(res == -1)
			{
				std::cout << "close failed" << std::endl;
			}
			boost::this_thread::sleep(boost::posix_time::milliseconds{1});
		}


		zmq_ctx_destroy(ctx);
		boost::this_thread::sleep(boost::posix_time::seconds{1000});
	}
	catch(const std::exception& e)
	{
		std::cout << "pub sub:" << e.what() << std::endl;
	}
	std::cout << "the end..." << std::endl;
}


BOOST_AUTO_TEST_SUITE_END()


