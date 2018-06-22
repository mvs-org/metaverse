/*
 * exception.cpp
 *
 *  Created on: Jun 5, 2017
 *      Author: jiang
 */

#include <jsoncpp/json/json.h>
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
    out << (fmt % (int32_t)ex.code() % ex.message());
    return out;
}

void relay_exception(std::stringstream& ss)
{
    try
    {
        Json::Reader reader;
        Json::Value root;
        if (reader.parse(ss, root) && root["code"].isUInt() && root["message"].isString()) {
            auto code = root["code"].asUInt();
            auto msg = root["message"].asString();
            if (code)
                throw explorer_exception{ code, msg };
        }
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
