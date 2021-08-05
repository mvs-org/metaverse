/**
 * Copyright (c) 2011-2021 libbitcoin developers (see AUTHORS)
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

#include <functional>
#include <iostream>
#include <utility>
#include <sstream>
#include <string>
#include <type_traits>
#include <mutex>
#include <boost/date_time.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/network/define.hpp>

namespace libbitcoin {

    // Guard against concurrent console writes.
    static std::mutex console_mutex;

    // Guard against concurrent file writes.
    static std::mutex file_mutex;

    template<class T>
    // TODO:limit template type of instance
    // class = typename std::enable_if<std::is_base_of<std::basic_ostream&, T>::value>::type>
    static inline void do_logging(T& ofs, log::level level, const std::string& domain,
        const std::string& body)
    {
        if (body.empty())
        {
            return;
        }

        namespace ptime = boost::posix_time;

        static const auto form = "%1% %2% [%3%] %4%\n";
        const auto message = boost::format(form) %
            ptime::to_iso_string(ptime::second_clock::local_time()) %
            log::to_text(level) %
            domain %
            body;

        {
            // Critical Section
            ///////////////////////////////////////////////////////////////////////
            std::unique_lock<std::mutex> lock_file(file_mutex);
            ofs << message;

            // Cut up log file if over max_size
            if (std::is_same<T, bc::ofstream>::value)
            {
                bc::ofstream& bo = dynamic_cast<bc::ofstream&>(ofs);
                auto& current_size = bo.current_size();
                current_size += message.size();
                if (bo.current_size() > bo.max_size())
                {
                    bo.close();
                    bo.open(bo.path(), std::ios::trunc | std::ios::out);
                    current_size = 0;
                }
            }

            ofs.flush();
            ///////////////////////////////////////////////////////////////////////

        }

}

static void output_ignore(bc::ofstream& file, log::level level,
    const std::string& domain, const std::string& body)
{

}

static void output_file(bc::ofstream& file, log::level level,
    const std::string& domain, const std::string& body)
{
    do_logging(file, level, domain, body);
}

static void output_both(bc::ofstream& file, std::ostream& output,
    log::level level, const std::string& domain, const std::string& body)
{
    do_logging(file, level, domain, body);
    do_logging(output, level, domain, body);
}

static void error_file(bc::ofstream& file, log::level level,
    const std::string& domain, const std::string& body)
{
    do_logging(file, level, domain, body);
}

static void error_both(bc::ofstream& file, std::ostream& error,
    log::level level, const std::string& domain, const std::string& body)
{
    do_logging(file, level, domain, body);
    do_logging(error, level, domain, body);
}

void initialize_logging(bc::ofstream& debug, bc::ofstream& error,
    std::ostream& output_stream, std::ostream& error_stream,
    std::string level)
{
    using namespace std::placeholders;

    auto debug_log_level = log::level::debug;
    if (level == "INFO" || level == "info")
        debug_log_level = log::level::info;
    else if (level == "TRACE" || level == "trace")
        debug_log_level = log::level::trace;

    // setup log level for debug_log
    if (debug_log_level < log::level::debug)
    {
        // trace|debug|info => debug_log
        log::trace("").set_output_function(std::bind(output_file,
            std::ref(debug), _1, _2, _3));
        log::debug("").set_output_function(std::bind(output_file,
            std::ref(debug), _1, _2, _3));
    }
    else if (debug_log_level < log::level::info)
    {
        log::trace("").set_output_function(std::bind(output_ignore,
            std::ref(debug), _1, _2, _3));
        // debug|info => debug_log
        log::debug("").set_output_function(std::bind(output_file,
            std::ref(debug), _1, _2, _3));
    }
    else if (debug_log_level < log::level::warning)
    {
        // info => debug_log
        log::trace("").set_output_function(std::bind(output_ignore,
            std::ref(debug), _1, _2, _3));
        log::debug("").set_output_function(std::bind(output_ignore,
            std::ref(debug), _1, _2, _3));
    }

    // info => debug_log + console
    log::info("").set_output_function(std::bind(output_both,
        std::ref(debug), std::ref(output_stream), _1, _2, _3));

    // warning|error|fatal => error_log + console
    log::warning("").set_output_function(std::bind(error_both,
        std::ref(error),std::ref(error_stream), _1, _2, _3));
    log::error("").set_output_function(std::bind(error_both,
        std::ref(error), std::ref(error_stream), _1, _2, _3));
    log::fatal("").set_output_function(std::bind(error_both,
        std::ref(error), std::ref(error_stream), _1, _2, _3));
}

} // namespace libbitcoin
