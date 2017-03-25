/*
 * Copyright (c) 2016-2017 metaverse core developers (see MVS-AUTHORS).
 * Copyright (C) 2013, 2016 Swirly Cloud Limited.
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
#include <metaverse/http/Exception_basic.hpp>

using namespace std;

namespace http {
namespace {
thread_local ErrMsg errMsg_;
} // anonymous

Exception::Exception(string_view what) noexcept
{
  const auto len = min(ErrMsgMax, what.size());
  if (len > 0) {
    memcpy(what_, what.data(), len);
  }
  what_[len] = '\0';
}

Exception::~Exception() noexcept = default;

const char* Exception::what() const noexcept
{
  return what_;
}

ErrMsg& errMsg() noexcept
{
  errMsg_.reset();
  return errMsg_;
}

} // http
