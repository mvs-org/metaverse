/*
 * backtrace.cpp
 *
 *  Created on: Nov 14, 2016
 *      Author: jiang
 */

#include <bitcoin/bitcoin/utility/backtrace.hpp>
#include <boost/format.hpp>
#include <fstream>
#ifndef _WIN32
#include <execinfo.h>
#endif


namespace libbitcoin
{
void back_trace(std::ostream& os)
{
#ifndef _WIN32
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

