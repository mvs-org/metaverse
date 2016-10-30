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
#ifndef LIBBITCOIN_NODE_DEFINE_HPP
#define LIBBITCOIN_NODE_DEFINE_HPP

#include <bitcoin/bitcoin.hpp>

// We use the generic helper definitions in libbitcoin to define BCN_API 
// and BCN_INTERNAL. BCN_API is used for the public API symbols. It either DLL
// imports or DLL exports (or does nothing for static build) BCN_INTERNAL is 
// used for non-api symbols.

#if defined BCN_STATIC
    #define BCN_API
    #define BCN_INTERNAL
#elif defined BCN_DLL
    #define BCN_API      BC_HELPER_DLL_EXPORT
    #define BCN_INTERNAL BC_HELPER_DLL_LOCAL
#else
    #define BCN_API      BC_HELPER_DLL_IMPORT
    #define BCN_INTERNAL BC_HELPER_DLL_LOCAL
#endif

// Log name.
#define LOG_NODE "node"

#endif
