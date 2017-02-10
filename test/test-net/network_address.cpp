/*
 * network_address.cpp
 *
 *  Created on: Dec 4, 2016
 *      Author: jiang
 */

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <bitcoin/bitcoin/message/network_address.hpp>
#include <bitcoin/bitcoin/config/authority.hpp>

BOOST_AUTO_TEST_SUITE(test_network_address)

struct F
{
	F(){

	}

	~F(){

	}
	libbitcoin::message::network_address address_;
};

static libbitcoin::message::network_address to_address(const std::string& ip, uint16_t port)
{
	libbitcoin::config::authority auth{ip, port};
	return auth.to_network_address();
}

BOOST_FIXTURE_TEST_CASE(test_is_valid, F)
{
	BOOST_CHECK(to_address("192.168.3.42", 7812).is_valid() );
	BOOST_CHECK(to_address("::1", 7812).is_private_network() );
	BOOST_CHECK(to_address("fe80::36de:1aff:fe0a:cc33", 7812).is_ipv6() );
	BOOST_CHECK(to_address("192.168.3.42", 7812).is_ipv4() );
	BOOST_CHECK(not to_address("192.168.3.42", 7812).is_ipv6() );
	BOOST_CHECK(not to_address("192.168.3.42", 7812).is_local() );
	BOOST_CHECK(not to_address("192.168.3.42", 7812).is_routable() );
	BOOST_CHECK(to_address("192.168.3.42", 7812).is_RFC1918() );
}

BOOST_AUTO_TEST_SUITE_END()



