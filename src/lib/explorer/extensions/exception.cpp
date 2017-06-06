/*
 * exception.cpp
 *
 *  Created on: Jun 5, 2017
 *      Author: jiang
 */

#include <metaverse/explorer/extensions/exception.hpp>

namespace libbitcoin {
namespace explorer {


explorer_exception::explorer_exception(uint32_t code, const std::string& message):
		code_{code}, message_{message}
{
}


} //namespace explorer
} //namespace libbitcoin
