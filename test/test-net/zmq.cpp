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

auto service_address = "inproc://test";

void create_sub(void* ctx)
{
	boost::thread th([ctx](){
		auto sub = zmq_socket(ctx, ZMQ_SUB);
		zmq_bind(sub, service_address);

		while(true)
		{
			char buf[1024] = {0};
			auto res = zmq_recv(sub,buf, 1024, 0);
			std::cout << "zmq recv result," << res << std::endl;
			if(res <= 0)
			{
				break;
			}
		}
	});
}

#include <boost/preprocessor.hpp>

#define CHECK_ZMQ_OPERATION(res, operation)\
		do{\
			if(res){\
				std::cout << operation << " failed" << std::endl;}\
		}while(0)

BOOST_AUTO_TEST_CASE(case_pub_sub)
{
	try
	{
		auto ctx = zmq_ctx_new();

#ifdef SUB_SERVICE_START
		create_sub(ctx);
#endif

		for(auto i=0;i<1025;++i)
		{
			auto res = 0;

			auto pub = zmq_socket(ctx, ZMQ_PUB);
			CHECK_ZMQ_OPERATION(!pub, "zmq socket");

			int value = 0;
			res = zmq_setsockopt(pub, ZMQ_LINGER, &value, sizeof(int));
			CHECK_ZMQ_OPERATION(res, "zmq setsocketopt");

			res = zmq_connect(pub, service_address);
			CHECK_ZMQ_OPERATION(res, "zmq connect");

			res = zmq_disconnect(pub, service_address);
			CHECK_ZMQ_OPERATION(res, "zmq disconnect");

			res = zmq_close(pub);
			CHECK_ZMQ_OPERATION(res, "zmq close");

			boost::this_thread::sleep(boost::posix_time::milliseconds{1});
		}

		boost::this_thread::sleep(boost::posix_time::seconds{1000});
		zmq_ctx_destroy(ctx);
	}
	catch(const std::exception& e)
	{
		std::cout << "pub sub:" << e.what() << std::endl;
	}
	std::cout << "the end..." << std::endl;
}


BOOST_AUTO_TEST_SUITE_END()


