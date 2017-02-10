/*
 * close_wait.cpp
 *
 *  Created on: Dec 26, 2016
 *      Author: jiang
 */

#include <boost/test/unit_test.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#if 0
using socket_ptr = boost::shared_ptr<boost::asio::ip::tcp::socket>;

void start_accept(boost::asio::ip::tcp::acceptor& acceptor_);
auto handle_accept = [](const boost::system::error_code& ec, socket_ptr sock, boost::asio::ip::tcp::acceptor& acceptor_){
	start_accept(acceptor_);
};

void start_accept(boost::asio::ip::tcp::acceptor& acceptor_)
{
	socket_ptr sock = boost::make_shared<boost::asio::ip::tcp::socket>(acceptor_.service() );
	acceptor_.async_accept(*sock, boost::bind(handle_accept, boost::asio::placeholders::error(), sock, boost::ref(acceptor_) ) );
}



BOOST_AUTO_TEST_SUITE(suit_close_wait)

BOOST_AUTO_TEST_CASE(case_check_post_in_run)
{
	boost::asio::io_service service_;
	boost::asio::io_service::work work_{service_};

	boost::asio::ip::tcp::endpoint endpoint_{boost::asio::ip::tcp::v4(), 1990};
	boost::asio::ip::tcp::acceptor acceptor_{service_, endpoint_};
	start_accept(acceptor_);

	service_.run();
}



BOOST_AUTO_TEST_CASE(case_close_wait)
{
#if 0
	boost::asio::io_service service_;
	boost::asio::io_service::work work_{service_};

	boost::asio::ip::tcp::endpoint endpoint_{boost::asio::ip::tcp::v4(), 1990};
	boost::asio::ip::tcp::acceptor acceptor_{service_, endpoint_};
	while(1)
	{
		boost::asio::ip::tcp::socket socket_{service_};
		boost::asio::ip::tcp::endpoint ep;
		boost::system::error_code ec;
		acceptor_.accept(socket_, ep, ec);

	}

	service_.run();
#endif
}

BOOST_AUTO_TEST_SUITE_END()
#endif
