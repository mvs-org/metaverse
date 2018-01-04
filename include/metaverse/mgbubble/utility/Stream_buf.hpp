/*
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS).
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
#ifndef MVSD_STREAM_HPP
#define MVSD_STREAM_HPP

#include "mongoose/mongoose.h"

#include <ostream>

/**
 * @addtogroup App
 * @{
 */

namespace mgbubble {

class StreamBuf : public std::streambuf {
 public:
  explicit StreamBuf(mbuf& buf) throw(std::bad_alloc);
  ~StreamBuf() noexcept override;

  // Copy.
  StreamBuf(const StreamBuf& rhs) = delete;
  StreamBuf& operator=(const StreamBuf& rhs) = delete;

  // Move.
  StreamBuf(StreamBuf&&) = delete;
  StreamBuf& operator=(StreamBuf&&) = delete;

  const char_type* data() const noexcept { return buf_.buf; }
  std::streamsize size() const noexcept { return buf_.len; }
  void reset() noexcept;
  void setContentLength(size_t pos, size_t len) noexcept;

 protected:
  int_type overflow(int_type c) noexcept override;

  std::streamsize xsputn(const char_type* s, std::streamsize count) noexcept override;

 private:
  mbuf& buf_;
};

class OStream : public std::ostream {
 public:
  OStream();
  ~OStream() noexcept override;

  // Copy.
  OStream(const OStream& rhs) = delete;
  OStream& operator=(const OStream& rhs) = delete;

  // Move.
  OStream(OStream&&) = delete;
  OStream& operator=(OStream&&) = delete;

  StreamBuf* rdbuf() const noexcept { return static_cast<StreamBuf*>(std::ostream::rdbuf()); }
  const char_type* data() const noexcept { return rdbuf()->data(); }
  std::streamsize size() const noexcept { return rdbuf()->size(); }
  StreamBuf* rdbuf(StreamBuf* sb) noexcept
  {
    return static_cast<StreamBuf*>(std::ostream::rdbuf(sb));
  }
  void reset(int status, const char* reason,const char *content_type = "text/plain",const char *charset = "utf-8") noexcept;
  void setContentLength() noexcept;

 private:
  size_t headSize_{0};
  size_t lengthAt_{0};
};

} // http

/** @} */

#endif // MVSD_STREAM_HPP
