/*
 * directories.hpp
 *
 *  Created on: Jan 8, 2017
 *      Author: jiang
 */

#ifndef INCLUDE_BITCOIN_BITCOIN_UTILITY_PATH_HPP_
#define INCLUDE_BITCOIN_BITCOIN_UTILITY_PATH_HPP_

#include <boost/filesystem.hpp>

namespace libbitcoin
{
	boost::filesystem::path default_data_path();

	boost::filesystem::path webpage_path();

}//namespace libbitcoin


#endif /* INCLUDE_BITCOIN_BITCOIN_UTILITY_PATH_HPP_ */
