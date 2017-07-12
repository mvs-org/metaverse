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

void relay_exception(std::stringstream& ss)
{
    std::stringstream sin;
    sin.str(ss.str());
    ss.str(""); // clear

    std::string code;
    std::string msg;
    // parse json
    using namespace boost::property_tree;
    try 
    {
        ptree pt;
        read_json(sin, pt);

        code = pt.get<std::string>("code");
        msg = pt.get<std::string>("message");
    }
    catch (...)
    {
        return;
    }
    uint32_t ex_code;
    ss << code;
    ss >> ex_code;
    ss.str(""); // clear
    throw explorer_exception{ex_code, msg};
}

} //namespace explorer
} //namespace libbitcoin
