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
#ifndef MVS___STRING_VIEW__
#define MVS___STRING_VIEW__

// Standard library support for std::string_view.
#if !defined(MVS_HAS_STD_STRING_VIEW)
# if !defined(MVS_DISABLE_STD_STRING_VIEW)
#  if defined(__clang__)
#   if (__cplusplus >= 201703)
#    if __has_include(<string_view>)
#     define MVS_HAS_STD_STRING_VIEW 1
#    endif // __has_include(<string_view>)
#   endif // (__cplusplus >= 201703)
#  endif // defined(__clang__)
#  if defined(__GNUC__)
#   if (__GNUC__ >= 7)
#    if (__cplusplus >= 201703)
#     define MVS_HAS_STD_STRING_VIEW 1
#    endif // (__cplusplus >= 201703)
#   endif // (__GNUC__ >= 7)
#  endif // defined(__GNUC__)
#  if defined(MVS_MSVC)
#   if (_MSC_VER >= 1910 && _HAS_CXX17)
#    define MVS_HAS_STD_STRING_VIEW
#   endif // (_MSC_VER >= 1910 && _HAS_CXX17)
#  endif // defined(MVS_MSVC)
# endif // !defined(MVS_DISABLE_STD_STRING_VIEW)
#endif // !defined(MVS_HAS_STD_STRING_VIEW)

// Standard library support for std::experimental::string_view.
// Note: libc++ 7.0 keeps <experimental/string_view> header but deprecates it
// with an #error directive.
#if !defined(MVS_HAS_STD_EXPERIMENTAL_STRING_VIEW)
# if !defined(MVS_DISABLE_STD_EXPERIMENTAL_STRING_VIEW)
#  if defined(__clang__)
#   if (__cplusplus >= 201402)
#    if __has_include(<string_view>)
#     define MVS_HAS_STD_STRING_VIEW 1
#    endif // __has_include(<string_view>)
#    if __has_include(<experimental/string_view>)
#     //if (__clang_major__ < 7)
#      define MVS_HAS_STD_EXPERIMENTAL_STRING_VIEW 1
#     //endif // if (__clang_major__ < 7)
#    endif // __has_include(<experimental/string_view>)
#   endif // (__cplusplus >= 201402)
#  endif // defined(__clang__)
#  if defined(__GNUC__)
#   if ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 9)) || (__GNUC__ > 4)
#    if (__cplusplus >= 201402)
#     define MVS_HAS_STD_EXPERIMENTAL_STRING_VIEW 1
#    endif // (__cplusplus >= 201402)
#   endif // ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 9)) || (__GNUC__ > 4)
#  endif // defined(__GNUC__)
# endif // !defined(MVS_DISABLE_STD_EXPERIMENTAL_STRING_VIEW)
#endif // !defined(MVS_HAS_STD_EXPERIMENTAL_STRING_VIEW)

#if MVS_HAS_STD_EXPERIMENTAL_STRING_VIEW
#include <experimental/string_view>
namespace std {
using string_view = std::experimental::string_view;
}
namespace mgbubble {
using std::string_view;
}

#else //MVS_HAS_STD_STRING_VIEW
#include <string_view>
namespace mgbubble {
using std::string_view;
}
#endif

#endif // MVS___STRING_VIEW__
