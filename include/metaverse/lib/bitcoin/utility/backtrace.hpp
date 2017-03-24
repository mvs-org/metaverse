/*
 * backtrace.hpp
 *
 *  Created on: Nov 14, 2016
 *      Author: jiang
 */

#ifndef INCLUDE_BITCOIN_BITCOIN_UTILITY_BACKTRACE_HPP_
#define INCLUDE_BITCOIN_BITCOIN_UTILITY_BACKTRACE_HPP_

#include <iostream>
#include <string>

namespace libbitcoin
{
void back_trace(std::ostream& os);
void do_backtrace(const std::string& name);
}//namespace libbitcoin

#endif /* INCLUDE_BITCOIN_BITCOIN_UTILITY_BACKTRACE_HPP_ */
