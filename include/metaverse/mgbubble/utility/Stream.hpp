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
#pragma once

#include <ostream>
#include <metaverse/mgbubble/compat/define.hpp>
#include <metaverse/mgbubble/compat/string_view.h>


/**
 * @addtogroup Util
 * @{
 */

namespace mgbubble {

MVS_API void reset(std::ostream& os) noexcept;

template <std::size_t MaxN>
class StringBuf : public std::streambuf {
 public:
  StringBuf() noexcept { reset(); }
  ~StringBuf() noexcept override = default;

  // Copy.
  StringBuf(const StringBuf& rhs) = delete;
  StringBuf& operator=(const StringBuf& rhs) = delete;

  // Move.
  StringBuf(StringBuf&&) = delete;
  StringBuf& operator=(StringBuf&&) = delete;

  const char* data() const noexcept { return pbase(); }
  bool empty() const noexcept { return pbase() == pptr(); }
  std::size_t size() const noexcept { return pptr() - pbase(); }
  string_view str() const noexcept { return {data(), size()}; }
  void reset() noexcept { setp(buf_, buf_ + MaxN); };
 private:
  char buf_[MaxN];
};

template <std::size_t MaxN>
class StringBuilder : public std::ostream {
 public:
  StringBuilder() : std::ostream{nullptr} { rdbuf(&buf_); }
  ~StringBuilder() noexcept override = default;

  // Copy.
  StringBuilder(const StringBuilder& rhs) = delete;
  StringBuilder& operator=(const StringBuilder& rhs) = delete;

  // Move.
  StringBuilder(StringBuilder&&) = delete;
  StringBuilder& operator=(StringBuilder&&) = delete;

  const char* data() const noexcept { return buf_.data(); }
  bool empty() const noexcept { return buf_.empty(); }
  std::size_t size() const noexcept { return buf_.size(); }
  string_view str() const noexcept { return buf_.str(); }
  operator string_view() const noexcept { return buf_.str(); }
  void reset() noexcept
  {
    buf_.reset();
    mgbubble::reset(*this);
  };

 private:
  StringBuf<MaxN> buf_;
};

template <std::size_t MaxN, typename ValueT>
auto& operator<<(StringBuilder<MaxN>& sb, ValueT&& val)
{
  static_cast<std::ostream&>(sb) << std::forward<ValueT>(val);
  return sb;
}

/**
 * Stream joiner. This is a simplified version of std::experimental::ostream_joiner, intended as a
 * placeholder until TS v2 is more widely available.
 */
class MVS_API OStreamJoiner {
 public:
  using char_type = char;
  using traits_type = std::char_traits<char_type>;
  using ostream_type = std::ostream;
  using value_type = void;
  using difference_type = void;
  using pointer = void;
  using reference = void;
  using iterator_category = std::output_iterator_tag;

  OStreamJoiner(std::ostream& os, const char delim) noexcept : os_{&os}, delim_{delim} {}
  ~OStreamJoiner() noexcept;

  // Copy.
  OStreamJoiner(const OStreamJoiner&) = default;
  OStreamJoiner& operator=(const OStreamJoiner&) = default;

  // Move.
  OStreamJoiner(OStreamJoiner&&) = default;
  OStreamJoiner& operator=(OStreamJoiner&&) = default;

  template <typename ValueT>
  OStreamJoiner& operator=(const ValueT& value)
  {
    if (!first_) {
      *os_ << delim_;
    }
    first_ = false;
    *os_ << value;
    return *this;
  }
  OStreamJoiner& operator*() noexcept { return *this; }
  OStreamJoiner& operator++() noexcept { return *this; }
  OStreamJoiner& operator++(int)noexcept { return *this; }

 private:
  std::ostream* os_;
  char delim_;
  bool first_{true};
};

} // http

/** @} */

