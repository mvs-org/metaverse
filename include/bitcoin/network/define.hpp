/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin-node.
 *
 * libbitcoin-node is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MVS_NETWORK_DEFINE_HPP
#define MVS_NETWORK_DEFINE_HPP

#include <bitcoin/bitcoin.hpp>

// We use the generic helper definitions in libbitcoin to define BCT_API 
// and BCT_INTERNAL. BCT_API is used for the public API symbols. It either DLL
// imports or DLL exports (or does nothing for static build) BCT_INTERNAL is 
// used for non-api symbols.

#if defined BCT_STATIC
    #define BCT_API
    #define BCT_INTERNAL
#elif defined BCT_DLL
    #define BCT_API      BC_HELPER_DLL_EXPORT
    #define BCT_INTERNAL BC_HELPER_DLL_LOCAL
#else
    #define BCT_API      BC_HELPER_DLL_IMPORT
    #define BCT_INTERNAL BC_HELPER_DLL_LOCAL
#endif

// Log name.
#define LOG_NETWORK "network"

// Avoid namespace conflict between boost::placeholders and std::placeholders. 
#define BOOST_BIND_NO_PLACEHOLDERS

// Include boost only here, so placeholders exclusion works.
#include <boost/asio.hpp>
#include <boost/circular_buffer.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/thread.hpp>

#endif
