/*
 * The NewReality Blockchain Project - Metaverse.
 * Copyright (C) 2016 viewfin.com.
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
#ifndef MVS_ASH_STRING_HPP
#define MVS_ASH_STRING_HPP

#include <mvs/http/Compare.hpp>
#include <mvs/http/Defs.hpp>

#include <experimental/string_view>

#include <cstring>
#include <sstream>

namespace std {
using experimental::string_view;
}

/**
 * @addtogroup Util
 * @{
 */

namespace http {

constexpr std::string_view operator""_sv(const char* str, std::size_t len) noexcept
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
constexpr std::string_view operator+(const StringData<MaxN>& s) noexcept
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
  constexpr String(std::string_view rhs) noexcept { assign(rhs.data(), rhs.size()); }
  constexpr String() noexcept { clear(); }

  ~String() noexcept = default;

  // Copy.
  constexpr String(const String& rhs) noexcept { assign(rhs.data(), rhs.size()); }
  constexpr String& operator=(const String& rhs) noexcept
  {
    assign(rhs.data(), rhs.size());
    return *this;
  }

  // Move.
  constexpr String(String&&) noexcept = default;
  constexpr String& operator=(String&&) noexcept = default;

  template <std::size_t MaxR>
  constexpr String& operator=(const String<MaxR>& rhs) noexcept
  {
    assign(rhs.data(), rhs.size());
    return *this;
  }
  constexpr String& operator=(std::string_view rhs) noexcept
  {
    assign(rhs.data(), rhs.size());
    return *this;
  }
  template <std::size_t MaxR>
  constexpr int compare(const String<MaxR>& rhs) const noexcept
  {
    return compare(rhs.data(), rhs.size());
  }
  constexpr int compare(std::string_view rhs) const noexcept
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
      result = http::compare(len_, rlen);
    }
    return result;
  }
  constexpr void assign(const char* rdata, std::size_t rlen) noexcept
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
constexpr std::string_view operator+(const String<MaxN>& s) noexcept
{
  return {s.data(), s.size()};
}

template <std::size_t MaxL, std::size_t MaxR>
constexpr bool operator==(const String<MaxL>& lhs, const String<MaxR>& rhs) noexcept
{
  return lhs.compare(rhs) == 0;
}

template <std::size_t MaxN>
constexpr bool operator==(const String<MaxN>& lhs, std::string_view rhs) noexcept
{
  return lhs.compare(rhs) == 0;
}

template <std::size_t MaxN>
constexpr bool operator==(std::string_view lhs, const String<MaxN>& rhs) noexcept
{
  return 0 == rhs.compare(lhs);
}

template <std::size_t MaxL, std::size_t MaxR>
constexpr bool operator!=(const String<MaxL>& lhs, const String<MaxR>& rhs) noexcept
{
  return lhs.compare(rhs) != 0;
}

template <std::size_t MaxN>
constexpr bool operator!=(const String<MaxN>& lhs, std::string_view rhs) noexcept
{
  return lhs.compare(rhs) != 0;
}

template <std::size_t MaxN>
constexpr bool operator!=(std::string_view lhs, const String<MaxN>& rhs) noexcept
{
  return 0 != rhs.compare(lhs);
}

template <std::size_t MaxL, std::size_t MaxR>
constexpr bool operator<(const String<MaxL>& lhs, const String<MaxR>& rhs) noexcept
{
  return lhs.compare(rhs) < 0;
}

template <std::size_t MaxN>
constexpr bool operator<(const String<MaxN>& lhs, std::string_view rhs) noexcept
{
  return lhs.compare(rhs) < 0;
}

template <std::size_t MaxN>
constexpr bool operator<(std::string_view lhs, const String<MaxN>& rhs) noexcept
{
  return 0 < rhs.compare(lhs);
}

template <std::size_t MaxL, std::size_t MaxR>
constexpr bool operator<=(const String<MaxL>& lhs, const String<MaxR>& rhs) noexcept
{
  return lhs.compare(rhs) <= 0;
}

template <std::size_t MaxN>
constexpr bool operator<=(const String<MaxN>& lhs, std::string_view rhs) noexcept
{
  return lhs.compare(rhs) <= 0;
}

template <std::size_t MaxN>
constexpr bool operator<=(std::string_view lhs, const String<MaxN>& rhs) noexcept
{
  return 0 <= rhs.compare(lhs);
}

template <std::size_t MaxL, std::size_t MaxR>
constexpr bool operator>(const String<MaxL>& lhs, const String<MaxR>& rhs) noexcept
{
  return lhs.compare(rhs) > 0;
}

template <std::size_t MaxN>
constexpr bool operator>(const String<MaxN>& lhs, std::string_view rhs) noexcept
{
  return lhs.compare(rhs) > 0;
}

template <std::size_t MaxN>
constexpr bool operator>(std::string_view lhs, const String<MaxN>& rhs) noexcept
{
  return 0 > rhs.compare(lhs);
}

template <std::size_t MaxL, std::size_t MaxR>
constexpr bool operator>=(const String<MaxL>& lhs, const String<MaxR>& rhs) noexcept
{
  return lhs.compare(rhs) >= 0;
}

template <std::size_t MaxN>
constexpr bool operator>=(const String<MaxN>& lhs, std::string_view rhs) noexcept
{
  return lhs.compare(rhs) >= 0;
}

template <std::size_t MaxN>
constexpr bool operator>=(std::string_view lhs, const String<MaxN>& rhs) noexcept
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

MVS_API unsigned long stoul(std::string_view sv) noexcept;

} // http

/** @} */

#endif // MVS_ASH_STRING_HPP
