/*
 * tcpwindowfull.cpp
 *
 *  Created on: Dec 2, 2016
 *      Author: jiang
 */


#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/make_shared.hpp>
#include <boost/crc.hpp>
#include <boost/date_time.hpp>


BOOST_AUTO_TEST_SUITE(suit_tcp_window_full)

struct F
{
	F()
	:service_{boost::thread::physical_concurrency()}
	{}
	~F(){}
	boost::asio::io_service service_;
};

int crc32(char *buf, std::size_t size)
{
	boost::crc_32_type crc;
	crc.process_bytes(buf, size);
	return crc.checksum();
}

using socket_ptr = boost::shared_ptr<boost::asio::ip::tcp::socket>;

class acceptor:public boost::enable_shared_from_this<acceptor>
{
public:
	using ptr = boost::shared_ptr<acceptor>;

	acceptor(boost::asio::io_service& service)
	: service_{service},
	  endpoint_{boost::asio::ip::tcp::v4(), 1990},
	  acceptor_{service},
	  count{0}
	{
		fill();
	}

	static void fill();

	void start()
	{
		std::cout << "address," << endpoint_ << std::endl;
		acceptor_.open(endpoint_.protocol());
		acceptor_.bind(endpoint_);
		acceptor_.listen();
		do_accept();
	}

	void do_accept()
	{
		auto sock = boost::make_shared<boost::asio::ip::tcp::socket>(service_) ;
		acceptor_.async_accept(*sock, boost::bind(&acceptor::handle_accept, this, boost::asio::placeholders::error(), sock));
	}

	void handle_accept(const boost::system::error_code& ec, socket_ptr sock)
	{
		if(!ec)
		{
			do_read(sock);
			do_accept();
			return;
		}
		std::cout << "async accept failed," << ec.message() << std::endl;
	}

	void do_read(socket_ptr sock)
	{
		constexpr auto size = 1024;
		auto buf = new char[1024];
		boost::asio::async_read(*sock, boost::asio::buffer(buf, size), boost::bind(&acceptor::handle_read, this, boost::asio::placeholders::error(), boost::asio::placeholders::bytes_transferred(), sock, buf));
	}

	void handle_read(const boost::system::error_code& ec, std::size_t byte_transafered, socket_ptr sock, char* buf)
	{
		if(ec)
		{
			std::cout << "handle read failed," << ec.message() << ",count," << count.load() << std::endl;
			sock->close();
			return;
		}

		auto checksum = crc32(buf, byte_transafered);
		delete[] buf;

		if(checksum != checksum_)
		{
			std::cerr << boost::posix_time::to_simple_string(boost::posix_time::second_clock::local_time()) << "checksum not equal," << checksum_ << ',' << checksum << "," << byte_transafered << std::endl;
			++count;
		}
		else
		{
//			std::cout << "same" << std::endl;
		}
//		std::cout << "================================" << std::endl;
		boost::this_thread::sleep(boost::posix_time::milliseconds{10});
		do_read(sock);
	}

	boost::asio::io_service& service_;
	boost::asio::ip::tcp::endpoint endpoint_;
	boost::asio::ip::tcp::acceptor acceptor_;
	std::atomic_int count;
	static char buf_[1024];
	static int checksum_;
};

char acceptor::buf_[1024];
int acceptor::checksum_;



void acceptor::fill()
{
	auto size = 1024;
	while(--size)
	{
		buf_[size] = size % 255;
	}
	checksum_ = crc32(buf_, 1024);
}

static void handle_write(const boost::system::error_code& ec, std::size_t byte_transferred, socket_ptr sock, std::atomic_int& count);

void do_write(socket_ptr sock, std::atomic_int& count)
{
	constexpr auto size = 1024;
	auto checksum = crc32(acceptor::buf_, size);
//	std::cout << i << "checksum," << checksum << std::endl;
	boost::asio::async_write(*sock, boost::asio::buffer(acceptor::buf_, size), boost::bind(&handle_write, boost::asio::placeholders::error(), boost::asio::placeholders::bytes_transferred(), sock, boost::ref(count)));
}

static void handle_write(const boost::system::error_code& ec, std::size_t byte_transferred, socket_ptr sock, std::atomic_int& count){
	if(ec)
	{
		std::cout << count.load() << ",async write failed," << ec.message() << std::endl;
		return;
	}
	++count;
	if(count == 100000)
	{
		std::cout << "write end" <<  std::endl;
		return;
	}
	do_write(sock, count);
}

void do_sync_write(socket_ptr sock)
{
	constexpr auto size = 1024;
	for(auto i = 0; i < 1000000; ++i)
	{
		boost::asio::async_write(*sock, boost::asio::buffer(acceptor::buf_, size), [](const boost::system::error_code& ec, std::size_t byte_transferred){
		});
	}
	std::cout << "write end" <<  std::endl;
}

BOOST_FIXTURE_TEST_CASE(case_tcp_window_full, F)
{
	acceptor acceptor_{service_};
	acceptor_.start();
	std::atomic_int count{0};
	service_.post([this, &count](){
//		while(true)
		{
			auto sock = boost::make_shared<boost::asio::ip::tcp::socket>(service_);
			auto address = boost::asio::ip::address::from_string("127.0.0.1");
			sock->async_connect(boost::asio::ip::tcp::endpoint{address, 1990}, [sock, this, &count](const boost::system::error_code& ec){
				if(!ec)
				{
					std::cout << "connected " <<  std::endl;
//					do_write(sock, count);
					do_sync_write(sock);
					return;
				}
				std::cout << "async connect failed," << ec.message() << std::endl;
			});
		}
	});

	service_.run();
}

BOOST_AUTO_TEST_SUITE_END()
