/*
 * exception.cpp
 *
 *  Created on: Jun 5, 2017
 *      Author: jiang
 */

#include <boost/property_tree/ptree.hpp>      
#include <boost/property_tree/json_parser.hpp>

#include <metaverse/bitcoin/error.hpp>
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

console_result capture_excode(std::stringstream& sstream, std::pair<uint32_t, std::string>& ex_pair)
{
	std::stringstream sin;
	sin.str(sstream.str());
	sstream.str(""); // clear
	// parse json
	using namespace boost::property_tree;
	try 
	{
		ptree pt;
		read_json(sin, pt);

		std::string code = pt.get<std::string>("code");
		if (code == "")
			return console_result::failure;
		std::string msg = pt.get<std::string>("message");
		std::stringstream ss;
		ss << code, ss >> ex_pair.first;
		ex_pair.second = msg;
		return console_result::okay;
	}
	catch (...)
	{
		return console_result::failure;
	}
}

} //namespace explorer
} //namespace libbitcoin
