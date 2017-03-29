/*
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS) - Metaverse.
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program; if
 * not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
#ifndef MVSD_EXCEPTION_HPP
#define MVSD_EXCEPTION_HPP

#include <metaverse/mgbubble/exception/Exception.hpp>

/**
 * @addtogroup App
 * @{
 */

namespace mgbubble {

class Error : public Exception {
 public:
  explicit Error(string_view what) noexcept : Exception{what} {}
  ~Error() noexcept = default;

  // Copy.
  Error(const Error&) noexcept = default;
  Error& operator=(const Error&) noexcept = default;

  // Move.
  Error(Error&&) noexcept = default;
  Error& operator=(Error&&) noexcept = default;
};

} // http

/** @} */

#endif // MVSD_EXCEPTION_HPP
