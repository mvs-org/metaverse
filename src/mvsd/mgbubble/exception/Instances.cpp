/*
 * Copyright (c) 2019-2020 metaverse core developers (see MVS-AUTHORS).
 * Copyright (c) 2019, 2020 Swirly Cloud Limited.
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
#include <metaverse/mgbubble/exception/Instances.hpp>

#include <iostream>

using namespace std;

namespace mgbubble {

ServException::~ServException() noexcept = default;

void ServException::toJson(int status, const char* reason, const char* detail, ostream& os)
{
  os << "{\"status\":" << status //
     << ",\"reason\":\"" << reason //
     << "\",\"detail\":\"" << detail //
     << "\"}";
}

BadRequestException::~BadRequestException() noexcept = default;

int BadRequestException::httpStatus() const noexcept
{
  return 400;
}

const char* BadRequestException::httpReason() const noexcept
{
  return "Bad Request";
}

AlreadyExistsException::~AlreadyExistsException() noexcept = default;

RefAlreadyExistsException::~RefAlreadyExistsException() noexcept = default;

InvalidException::~InvalidException() noexcept = default;

ForbiddenException::~ForbiddenException() noexcept = default;

int ForbiddenException::httpStatus() const noexcept
{
  return 403;
}

const char* ForbiddenException::httpReason() const noexcept
{
  return "Forbidden";
}

InternalException::~InternalException() noexcept = default;

int InternalException::httpStatus() const noexcept
{
  return 500;
}

const char* InternalException::httpReason() const noexcept
{
  return "Internal Server Error";
}

MethodNotAllowedException::~MethodNotAllowedException() noexcept = default;

int MethodNotAllowedException::httpStatus() const noexcept
{
  return 405;
}

const char* MethodNotAllowedException::httpReason() const noexcept
{
  return "Method Not Allowed";
}

NotFoundException::~NotFoundException() noexcept = default;

int NotFoundException::httpStatus() const noexcept
{
  return 404;
}

const char* NotFoundException::httpReason() const noexcept
{
  return "Not Found";
}

ServiceUnavailableException::~ServiceUnavailableException() noexcept = default;

int ServiceUnavailableException::httpStatus() const noexcept
{
  return 503;
}

const char* ServiceUnavailableException::httpReason() const noexcept
{
  return "Service Unavailable";
}

UnauthorizedException::~UnauthorizedException() noexcept = default;

int UnauthorizedException::httpStatus() const noexcept
{
  return 401;
}

const char* UnauthorizedException::httpReason() const noexcept
{
  return "Unauthorized";
}

} // mgbubble
