/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS)
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

#include <functional>
#include <iostream>
#include <utility>
#include <sstream>
#include <string>
#include <boost/date_time.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/define.hpp>
#include <mutex>

namespace libbitcoin {

// Guard against concurrent console writes.
static std::mutex console_mutex;

// Guard against concurrent file writes.
static std::mutex file_mutex;

template<class T>
//		 class = typename std::enable_if<std::is_base_of<std::basic_ostream&, T>::value>::type>
static inline void do_logging(T& ofs, log::level level, const std::string& domain,
    const std::string& body)
{
    if (body.empty())
    {
        return;
    }

	static uint8_t flush_times = 0;
    static const auto form = "%1% %2% [%3%] %4%\n";
    const auto message = boost::format(form) %
        boost::posix_time::microsec_clock::local_time().time_of_day() %
        log::to_text(level) %
        domain %
        body;

	{
        // Critical Section
        ///////////////////////////////////////////////////////////////////////
    	std::unique_lock<std::mutex> lock_file(file_mutex);
		ofs<<message;
        ofs.flush();
        ///////////////////////////////////////////////////////////////////////
	}

}
#if 0
static void log_to_file(std::ofstream& file, log::level level,
    const std::string& domain, const std::string& body)
{
	do_logging(file, level, domain, body);	
}

static void log_to_both(std::ostream& device, std::ofstream& file,
    log::level level, const std::string& domain, const std::string& body)
{
	do_logging(file, level, domain, body);	
	do_logging(device, level, domain, body);	
}
#endif
static void output_ignore(std::ofstream& file, log::level level,
    const std::string& domain, const std::string& body)
{

}

static void output_file(std::ofstream& file, log::level level,
    const std::string& domain, const std::string& body)
{
	do_logging(file, level, domain, body);	
    //log_to_file(file, level, domain, body);
}

static void output_both(std::ofstream& file, std::ostream& output,
    log::level level, const std::string& domain, const std::string& body)
{
    //log_to_both(output, file, level, domain, body);
	do_logging(file, level, domain, body);	
	do_logging(output, level, domain, body);	
}

static void error_file(std::ofstream& file, log::level level,
    const std::string& domain, const std::string& body)
{
    //log_to_file(file, level, domain, body);
	do_logging(file, level, domain, body);	
}

static void error_both(std::ofstream& file, std::ostream& error,
    log::level level, const std::string& domain, const std::string& body)
{
    //log_to_both(error, file, level, domain, body);
	do_logging(file, level, domain, body);	
	do_logging(error, level, domain, body);	
}

void initialize_logging(std::ofstream& debug, std::ofstream& error,
    std::ostream& output_stream, std::ostream& error_stream)
{
    using namespace std::placeholders;

	// trace => debug_log if define MVS_DEBUG
	#ifdef MVS_DEBUG
    log::trace("").set_output_function(std::bind(output_file,
        std::ref(debug), _1, _2, _3));
	#else
    log::trace("").set_output_function(std::bind(output_ignore,
        std::ref(debug), _1, _2, _3));
	#endif
	
    // debug|info => debug_log
    log::debug("").set_output_function(std::bind(output_file,
        std::ref(debug), _1, _2, _3));
    log::info("").set_output_function(std::bind(output_both,
        std::ref(debug), std::ref(output_stream), _1, _2, _3));

    // warning|error|fatal => error_log + console
    log::warning("").set_output_function(std::bind(error_file,
        std::ref(error), _1, _2, _3));
    log::error("").set_output_function(std::bind(error_both,
        std::ref(error), std::ref(error_stream), _1, _2, _3));
    log::fatal("").set_output_function(std::bind(error_both,
        std::ref(error), std::ref(error_stream), _1, _2, _3));
}

} // namespace libbitcoin
