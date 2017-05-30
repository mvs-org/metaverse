/*
 * session_tests.cpp
 *
 *  Created on: Dec 4, 2016
 *      Author: jiang
 */

#include <iostream>
#include <memory>
#include <boost/test/unit_test.hpp>
#include <metaverse/network/sessions/session_inbound.hpp>
#include <metaverse/network/sessions/session_outbound.hpp>
#include <metaverse/network/sessions/session_manual.hpp>
#include <metaverse/network/p2p.hpp>
#include <future>

std::promise<libbitcoin::code> complete;

BOOST_AUTO_TEST_SUITE(test_session)

using namespace libbitcoin;

class manual_session : public network::session_manual
{
public:
	typedef std::function<void(const code&)> result_handler;
	using ptr = std::shared_ptr<manual_session>;
	manual_session(network::p2p& network)
	:network::session_manual{network}
	{
	}

	void start(result_handler handler)
	{
//		handler(error::success);
	}
};

class jnode : public network::p2p
{
public:
	using ptr = std::shared_ptr<jnode>;
	jnode(const network::settings& settings):
		network::p2p(settings)
	{

	}
	virtual void start(result_handler handler)
	{
		store(config::authority{"192.168.3.42", 8812}.to_network_address(), [this, handler](const code& ec){
			std::cout << "store result," << ec.message() << std::endl;
			network::p2p::start(handler);
		});
		handler(error::success);
	}

	network::session_manual::ptr attach_manual_session()
	{
		return attach<manual_session>();
	}
};
static void signal_handler(int s)
{
	std::cout << "receive signal," << s << std::endl;
	complete.set_value(code{error::success});
}
struct F
{
	F()
	 :result{initial_logging()},
	  settings_{libbitcoin::settings::mainnet},
	  network_{new jnode(settings_)}
	{
		signal(SIGINT, signal_handler);
		settings_.seeds.clear();
	}
	static std::ofstream fout;
	static bool initial_logging()
	{

		fout.open("123", std::ios_base::app);
		if(not fout.good())
		{
			throw std::runtime_error{"open file failed"};
		}
		initialize_logging(fout, fout, std::cout, std::cout);
		return true;
	}

	~F(){
	}
	bool result;
	network::settings settings_;
	jnode::ptr network_;

};
std::ofstream F::fout;

BOOST_FIXTURE_TEST_CASE(case_session_outbound, F)
{

	std::cout << "begin to start..." << std::endl;

	auto pThis = network_;
	network_->start([pThis](const libbitcoin::code& ec){

		std::cout << "node started" << std::endl;

		auto outbound = std::make_shared<network::session_outbound>(*pThis);

		outbound->start([](const libbitcoin::code& ec){
			std::cout << "out bound session start result:" << ec.message() << std::endl;
		});

	});

	complete.get_future().wait();
	network_->stop();
}



BOOST_AUTO_TEST_SUITE_END()

