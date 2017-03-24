/*
 * time.cpp
 *
 *  Created on: Jan 3, 2017
 *      Author: jiang
 */
#include <metaverse/lib/bitcoin/utility/time.hpp>
#include <chrono>

namespace libbitcoin { //namespace libbitcoin

int64_t unix_millisecond()
{
	using namespace std::chrono;
	return system_clock::now().time_since_epoch().count();
}

}//namespace libbitcoin
