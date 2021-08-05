/**
 * Copyright (c) 2016-2021 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse.
 *
 * metaverse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/bitcoin/utility/backtrace.hpp>
#include <boost/format.hpp>
#include <fstream>
#ifndef _WIN32
#ifndef __ANDROID__
#include <execinfo.h>
#endif
#endif


namespace libbitcoin
{
void back_trace(std::ostream& os)
{
#ifndef _WIN32
#ifndef __ANDROID__
    int size = 1024;
    int j = 0, nptrs = 0;

    void *buffer[1024];
    char **strings;

    nptrs = backtrace(buffer, size);
    os << "backtrace() returned " << nptrs << " addresses\n";

    strings = backtrace_symbols(buffer, nptrs);
    if (strings == nullptr) {
        os << "backtrace_symbols failed" << '\n';
        exit (EXIT_FAILURE);
    }

    for (j = 0; j < nptrs; j++)
    {
        os << strings[j] << '\n';
    }

    free(strings);
#endif
#endif
}


void do_backtrace(const std::string& name)
{
    std::fstream fout;
    fout.open(name, std::ios_base::ate|std::ios_base::out);
    if(! fout.good())
    {
        boost::format fmt{"open file %s failed"};
        auto msg = fmt % name;
        throw std::runtime_error{ msg.str() };
    }
    back_trace(fout);
}

}//namespace libbitcoin

