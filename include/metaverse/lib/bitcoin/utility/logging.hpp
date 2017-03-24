/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
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
#ifndef MVS_NETWORK_LOGGING_HPP
#define MVS_NETWORK_LOGGING_HPP

#include <fstream>
#include <iostream>
#include <metaverse/lib/bitcoin.hpp>
#include <metaverse/lib/network/define.hpp>

namespace libbitcoin {

/// Set up global logging.
BCT_API void initialize_logging(std::ofstream& debug, std::ofstream& error,
    std::ostream& output_stream, std::ostream& error_stream);

/// Class Logger
class Logger{
#define self Logger

public:
    self() noexcept
    {
        initialize_logging(debug_log_, error_log_, std::cout, std::cerr);
    }

	self(const self&) = delete;
	self(const self&&) = delete;

    ~self() noexcept
    {
        log::clear();
		debug_log_.close();
		error_log_.close();
    }

public:
/// Constant for logging file open mode (append output).
static BC_CONSTEXPR std::ofstream::openmode log_open_mode =
    std::ofstream::out | std::ofstream::app;

private:
    std::ofstream debug_log_{"debug.log", log_open_mode};
    std::ofstream error_log_{"error.log", log_open_mode};

#undef self
};// class Logger

} // namespace libbitcoin

#endif
