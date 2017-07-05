/*
 * exception.cpp
 *
 *  Created on: Jun 5, 2017
 *      Author: jiang
 */

#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {

explorer_exception::explorer_exception(uint32_t code, const std::string& message) :
		code_{code}, message_{message}
{
}

std::ostream& operator<<(std::ostream& out, const explorer_exception& ex)
{
	boost::format fmt{"{\"code\":%d, \"message\":\"%s\", \"result\":null}"};
	out << (fmt % ex.code() % ex.message());
	return out;
}

} //namespace explorer
} //namespace libbitcoin
