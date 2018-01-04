/*
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS) - Metaverse.
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

#pragma once

#include <metaverse/mgbubble/utility/String.hpp>

#include <algorithm>

/**
 * @addtogroup Util
 * @{
 */

namespace mgbubble {

template <char DelimN>
class Tokeniser {
 public:
  explicit Tokeniser(string_view buf) noexcept { reset(buf); }
  explicit Tokeniser() noexcept { reset(""); }
  ~Tokeniser() noexcept = default;

  // Copy.
  Tokeniser(const Tokeniser& rhs) noexcept = default;
  Tokeniser& operator=(const Tokeniser& rhs) noexcept = default;

  // Move.
  Tokeniser(Tokeniser&&) noexcept = default;
  Tokeniser& operator=(Tokeniser&&) noexcept = default;

  string_view top() const noexcept { return buf_.substr(i_ - buf_.cbegin(), j_ - i_); }
  bool empty() const noexcept { return i_ == buf_.cend(); }
  void reset(string_view buf) noexcept
  {
    buf_ = buf;
    i_ = buf_.cbegin();
    j_ = std::find(i_, buf_.cend(), DelimN);
  }
  void pop() noexcept
  {
    if (j_ != buf_.cend()) {
      i_ = j_ + 1;
      j_ = std::find(i_, buf_.cend(), DelimN);
    } else {
      i_ = j_;
    }
  }

 private:
  string_view buf_;
  string_view::const_iterator i_, j_;
};

} // http

/** @} */
