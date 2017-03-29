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

#include <cstring>
#include <sstream>

#include <metaverse/mgbubble/compat/define.hpp>
#include <metaverse/mgbubble/compat/string_view.h>
#include <metaverse/mgbubble/utility/Compare.hpp>


/**
 * @addtogroup Util
 * @{
 */

namespace mgbubble {

constexpr string_view operator""_sv(const char* str, std::size_t len) noexcept
{
  return {str, len};
}

template <std::size_t MaxN>
struct StringData {

  // Length in the first cache-line.
  std::size_t len;
  char buf[MaxN];
};

template <std::size_t MaxN>
constexpr string_view operator+(const StringData<MaxN>& s) noexcept
{
  return {s.buf, s.len};
}

/**
 * String buffer with fixed upper-bound.
 */
template <std::size_t MaxN>
class String {
 public:
  template <std::size_t MaxR>
  constexpr String(const String<MaxR>& rhs) noexcept
  {
    assign(rhs.data(), rhs.size());
  }
  constexpr String(string_view rhs) noexcept { assign(rhs.data(), rhs.size()); }
  constexpr String() noexcept { clear(); }

  ~String() noexcept = default;

  // Copy.
  constexpr String(const String& rhs) noexcept { assign(rhs.data(), rhs.size()); }
  constexpr String& operator=(const String& rhs) const noexcept
  {
    assign(rhs.data(), rhs.size());
    return *this;
  }

  // Move.
  constexpr String(String&&) noexcept = default;
  String& operator=(String&&) noexcept = default;

  template <std::size_t MaxR>
  constexpr String& operator=(const String<MaxR>& rhs) const noexcept
  {
    assign(rhs.data(), rhs.size());
    return *this;
  }
  constexpr String& operator=(string_view rhs) const noexcept
  {
    assign(rhs.data(), rhs.size());
    return *this;
  }
  template <std::size_t MaxR>
  constexpr int compare(const String<MaxR>& rhs) const noexcept
  {
    return compare(rhs.data(), rhs.size());
  }
  constexpr int compare(string_view rhs) const noexcept
  {
    return compare(rhs.data(), rhs.size());
  }
  constexpr const char* data() const noexcept { return buf_; }
  constexpr bool empty() const noexcept { return len_ == 0; }
  constexpr size_t size() const noexcept { return len_; }
  constexpr void clear() noexcept { len_ = 0; }

 private:
  constexpr int compare(const char* rdata, std::size_t rlen) const noexcept
  {
    int result{std::memcmp(buf_, rdata, std::min(len_, rlen))};
    if (result == 0) {
      result = mgbubble::compare(len_, rlen);
    }
    return result;
  }
  constexpr void assign(const char* rdata, std::size_t rlen) const noexcept
  {
    len_ = std::min(MaxN, rlen);
    if (len_ > 0) {
      std::memcpy(buf_, rdata, len_);
    }
  }
  // Length in the first cache-line.
  std::size_t len_;
  char buf_[MaxN];
};

template <std::size_t MaxN>
constexpr string_view operator+(const String<MaxN>& s) noexcept
{
  return {s.data(), s.size()};
}

template <std::size_t MaxL, std::size_t MaxR>
constexpr bool operator==(const String<MaxL>& lhs, const String<MaxR>& rhs) noexcept
{
  return lhs.compare(rhs) == 0;
}

template <std::size_t MaxN>
constexpr bool operator==(const String<MaxN>& lhs, string_view rhs) noexcept
{
  return lhs.compare(rhs) == 0;
}

template <std::size_t MaxN>
constexpr bool operator==(string_view lhs, const String<MaxN>& rhs) noexcept
{
  return 0 == rhs.compare(lhs);
}

template <std::size_t MaxL, std::size_t MaxR>
constexpr bool operator!=(const String<MaxL>& lhs, const String<MaxR>& rhs) noexcept
{
  return lhs.compare(rhs) != 0;
}

template <std::size_t MaxN>
constexpr bool operator!=(const String<MaxN>& lhs, string_view rhs) noexcept
{
  return lhs.compare(rhs) != 0;
}

template <std::size_t MaxN>
constexpr bool operator!=(string_view lhs, const String<MaxN>& rhs) noexcept
{
  return 0 != rhs.compare(lhs);
}

template <std::size_t MaxL, std::size_t MaxR>
constexpr bool operator<(const String<MaxL>& lhs, const String<MaxR>& rhs) noexcept
{
  return lhs.compare(rhs) < 0;
}

template <std::size_t MaxN>
constexpr bool operator<(const String<MaxN>& lhs, string_view rhs) noexcept
{
  return lhs.compare(rhs) < 0;
}

template <std::size_t MaxN>
constexpr bool operator<(string_view lhs, const String<MaxN>& rhs) noexcept
{
  return 0 < rhs.compare(lhs);
}

template <std::size_t MaxL, std::size_t MaxR>
constexpr bool operator<=(const String<MaxL>& lhs, const String<MaxR>& rhs) noexcept
{
  return lhs.compare(rhs) <= 0;
}

template <std::size_t MaxN>
constexpr bool operator<=(const String<MaxN>& lhs, string_view rhs) noexcept
{
  return lhs.compare(rhs) <= 0;
}

template <std::size_t MaxN>
constexpr bool operator<=(string_view lhs, const String<MaxN>& rhs) noexcept
{
  return 0 <= rhs.compare(lhs);
}

template <std::size_t MaxL, std::size_t MaxR>
constexpr bool operator>(const String<MaxL>& lhs, const String<MaxR>& rhs) noexcept
{
  return lhs.compare(rhs) > 0;
}

template <std::size_t MaxN>
constexpr bool operator>(const String<MaxN>& lhs, string_view rhs) noexcept
{
  return lhs.compare(rhs) > 0;
}

template <std::size_t MaxN>
constexpr bool operator>(string_view lhs, const String<MaxN>& rhs) noexcept
{
  return 0 > rhs.compare(lhs);
}

template <std::size_t MaxL, std::size_t MaxR>
constexpr bool operator>=(const String<MaxL>& lhs, const String<MaxR>& rhs) noexcept
{
  return lhs.compare(rhs) >= 0;
}

template <std::size_t MaxN>
constexpr bool operator>=(const String<MaxN>& lhs, string_view rhs) noexcept
{
  return lhs.compare(rhs) >= 0;
}

template <std::size_t MaxN>
constexpr bool operator>=(string_view lhs, const String<MaxN>& rhs) noexcept
{
  return 0 >= rhs.compare(lhs);
}

template <std::size_t MaxN>
constexpr std::ostream& operator<<(std::ostream& os, const String<MaxN>& rhs) noexcept
{
  return os << +rhs;
}

template <typename ValueT, typename std::enable_if_t<std::is_arithmetic<ValueT>::value>* = nullptr>
std::string toString(ValueT val)
{
  return std::to_string(val);
}

template <typename ValueT, typename std::enable_if_t<!std::is_arithmetic<ValueT>::value>* = nullptr>
std::string toString(const ValueT& val)
{
  std::stringstream ss;
  ss << val;
  return ss.str();
}

MVS_API unsigned long stoul(string_view sv) noexcept;

} // http

/** @} */

