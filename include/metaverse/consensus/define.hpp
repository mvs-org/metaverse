/**
 * Copyright (c) 2011-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2020 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse-consensus.
 *
 * metaverse-consensus is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
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
#ifndef MVS_CONSENSUS_DEFINE_HPP
#define MVS_CONSENSUS_DEFINE_HPP

// See http://gcc.gnu.org/wiki/Visibility

// Generic helper definitions for shared library support
#if defined _MSC_VER || defined __CYGWIN__
    #define BCK_HELPER_DLL_IMPORT __declspec(dllimport)
    #define BCK_HELPER_DLL_EXPORT __declspec(dllexport)
    #define BCK_HELPER_DLL_LOCAL
#else
    #if __GNUC__ >= 4
        #define BCK_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
        #define BCK_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
        #define BCK_HELPER_DLL_LOCAL  __attribute__ ((visibility ("internal")))
    #else
        #define BCK_HELPER_DLL_IMPORT
        #define BCK_HELPER_DLL_EXPORT
        #define BCK_HELPER_DLL_LOCAL
    #endif
#endif

// Now we use the generic helper definitions above to
// define BCK_API and BCK_INTERNAL.
// BCK_API is used for the public API symbols. It either DLL imports or
// DLL exports (or does nothing for static build)
// BCK_INTERNAL is used for non-api symbols.

#if defined BCK_STATIC
    #define BCK_API
    #define BCK_INTERNAL
#elif defined BCK_DLL
    #define BCK_API      BCK_HELPER_DLL_EXPORT
    #define BCK_INTERNAL BCK_HELPER_DLL_LOCAL
#else
    #define BCK_API      BCK_HELPER_DLL_IMPORT
    #define BCK_INTERNAL BCK_HELPER_DLL_LOCAL
#endif

#endif
