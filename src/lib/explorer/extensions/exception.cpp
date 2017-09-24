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
    boost::format fmt{"{\"code\":%d, \"error\":\"%s\", \"result\":null}"};
    out << (fmt % ex.code() % ex.message());
    return out;
}

void relay_exception(std::stringstream& ss)
{    
    // parse json
    using namespace boost::property_tree;
    try 
    {
        ptree pt;
        read_json(ss, pt);
        uint32_t code = pt.get<uint32_t>("code");
        std::string msg = pt.get<std::string>("message");
        if (code) 
            return;
        throw explorer_exception{code, msg};
    }
    catch (const explorer_exception& e)
    {
        throw e;
    }
    catch (const std::exception& e)
    {
    }
}

} //namespace explorer
} //namespace libbitcoin
