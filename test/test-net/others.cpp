/*
 * others.cpp
 *
 *  Created on: Dec 5, 2016
 *      Author: jiang
 */

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <vector>
#include <metaverse/bitcoin/chain/script/script.hpp>
#include <metaverse/bitcoin/formats/base_58.hpp>
#include <boost/asio.hpp>
#include <fstream>
#include <boost/thread.hpp>
#include <metaverse/bitcoin/formats/base_85.hpp>
#include <metaverse/bitcoin/utility/daemon.hpp>

BOOST_AUTO_TEST_SUITE(suit_others)

BOOST_AUTO_TEST_CASE(case_script)
{
	std::string pubkey = "5KQPgu989hkPGrRd8NxiwGubdzxQpW5uuybRPirzJTqwwj3kxtM";

	libbitcoin::data_chunk buffer;
	libbitcoin::decode_base58(buffer, pubkey);
	std::string decoded_pubkey{buffer.begin(), buffer.end()};
	std::cout << decoded_pubkey << std::endl;
}


template <typename... Args>
class D
{
public:
	using handle = std::function<void (Args...)>;
	using handles = std::vector<handle>;
	void push_back(handle h)
	{
		handles_.push_back(h);
	}
private:
	handles handles_;
};


BOOST_AUTO_TEST_CASE(case_vector_template_parameter)
{
	using namespace std;
//	1  >> std::cout ;
	D<int, std::string> d;
	d.push_back([](int, std::string){

	});
}

boost::asio::coroutine c;

void foo()
{

}

BOOST_AUTO_TEST_CASE(case_coroutine)
{

}

BOOST_AUTO_TEST_CASE(case_touch_file_umask)
{
	umask(0000);
	std::ofstream fout;
	fout.open("123", std::ios_base::ate);
}

void sleep_for(long sleep)
{
	boost::this_thread::sleep(boost::posix_time::seconds{sleep});
}


BOOST_AUTO_TEST_CASE(case_strand)
{
	boost::thread_group group_;
	boost::asio::io_service service_;
	boost::asio::io_service::strand strand_{service_};
	boost::asio::io_service::work work_{service_};
	for(auto i=0;i<2;++i)
	{
		group_.create_thread(boost::bind(&boost::asio::io_service::run, &service_));
	}
	for(auto i=0;i<10;++i)
	{
		strand_.post([&service_, i](){
			std::cout << "before strand work," << i << std::endl;
			if (i == 6) {
				std::cout << "post a work to service" << std::endl;
				service_.post([](){
					std::cout << "service work" << std::endl;
					sleep_for(1);
				});
			}
			sleep_for(1);
			std::cout << "after strand work," << i << std::endl;
		});
	}

	group_.join_all();
}


BOOST_AUTO_TEST_CASE(case_encode_base58){
	std::string out;
	unsigned char hash[25] = {0};
	libbitcoin::data_slice s{std::begin(hash), std::end(hash)};
	out = libbitcoin::encode_base58(s);
	std::cout << "address:" << out << std::endl;
}

BOOST_AUTO_TEST_CASE(case_daemon){
	libbitcoin::daemon();
	std::cout << "after daemon"<< std::endl;
	std::fstream fout;
	fout.open("123", std::ios_base::out);
	fout.close();
}


BOOST_AUTO_TEST_CASE(case_fork){
	int pid = fork();
	if(pid < 0)
	{
		std::cout << "fork failed" << std::endl;
		return;
	}

	if(pid == 0)
	{
		std::cout << "child" << std::endl;
		sleep(1000000);
	}
	else
	{
		std::cout << "father" << std::endl;
		sleep(1000000);
	}
}

BOOST_AUTO_TEST_CASE(case_syn_sent){
	boost::asio::io_service service_;
	boost::asio::ip::tcp::socket sock{service_};
	boost::asio::ip::tcp::endpoint endpoint{boost::asio::ip::address::from_string("139.129.217.27"), 52510};
	boost::system::error_code ec;
#ifdef  SYNC
	{
		sock.connect(endpoint, ec);
		if(ec)
		{
			std::cout << "connect failed," << ec.message() << std::endl;
		}
		sleep(10);
		std::cout << "socket closed"<< std::endl;
		sock.close();
		sleep(10000000);
	}
#else
	sock.async_connect(endpoint, [](const boost::system::error_code& ec){
		if(ec)
		{
			std::cout << "connect failed," << ec.message() << std::endl;
			sleep(10000000);
		}
		else
		{
			std::cout << "connect success"<< std::endl;
		}
	});
#endif
	service_.run();
}



BOOST_AUTO_TEST_SUITE_END()



