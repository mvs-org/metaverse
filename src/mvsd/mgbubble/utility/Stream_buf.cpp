/*
 * Copyright (c) 2019-2020 metaverse core developers (see MVS-AUTHORS).
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

#include <metaverse/mgbubble/utility/Stream.hpp>
#include <metaverse/mgbubble/utility/Stream_buf.hpp>
#include <metaverse/mgbubble/utility/String.hpp>

using namespace std;

namespace mgbubble {

StreamBuf::StreamBuf(mbuf& buf) : buf_(buf)
{
  if (!buf_.buf) {
    // Pre-allocate buffer.
    mbuf_init(&buf_, 4096);
    if (!buf_.buf) {
      throw bad_alloc();
    }
  }
}

StreamBuf::~StreamBuf() noexcept = default;

void StreamBuf::reset() noexcept
{
  buf_.len = 0;
}

void StreamBuf::setContentLength(size_t pos, size_t len) noexcept
{
  char* ptr{buf_.buf + pos};
  do {
    --ptr;
    *ptr = '0' + len % 10;
    len /= 10;
  } while (len > 0);
}

StreamBuf::int_type StreamBuf::overflow(int_type c) noexcept
{
  if (c != traits_type::eof()) {
    const char z = c;
    if (mbuf_append(&buf_, &z, 1) != 1) {
      c = traits_type::eof();
    }
  }
  return c;
}

streamsize StreamBuf::xsputn(const char_type* s, streamsize count) noexcept
{
  return mbuf_append(&buf_, s, count);
}

OStream::OStream() : ostream{nullptr}
{
}

OStream::~OStream() noexcept = default;

void OStream::reset(int status, const char* reason,const char *content_type,const char *charset) noexcept
{
  rdbuf()->reset();
  mgbubble::reset(*this);

  // Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF. Use 10 space place-holder for
  // content length. RFC2616 states that field value MAY be preceded by any amount of LWS, though a
  // single SP is preferred.
  *this << "HTTP/1.1 " << status << ' ' << reason << "\r\nContent-Type: "<< content_type<<";charset="<<charset<<"\r\nContent-Length:           \r\n\r\n";
  headSize_ = size();
  lengthAt_ = headSize_ - 4;
}

void OStream::setContentLength() noexcept
{
  rdbuf()->setContentLength(lengthAt_, size() - headSize_);
}

} // mgbubble
