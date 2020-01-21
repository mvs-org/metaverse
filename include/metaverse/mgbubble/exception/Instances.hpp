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
#pragma once

#include <iosfwd>
#include <metaverse/mgbubble/exception/Exception.hpp>


/**
 * @addtogroup Exception
 * @{
 */

namespace mgbubble {

class MVS_API ServException : public Exception {
public:
    explicit ServException(string_view what) noexcept : Exception{what} {}
    ~ServException() noexcept override;

    // Copy.
    ServException(const ServException&) noexcept = default;
    ServException& operator=(const ServException&) noexcept = default;

    // Move.
    ServException(ServException&&) noexcept = default;
    ServException& operator=(ServException&&) noexcept = default;

    static void toJson(int status, const char* reason, const char* detail, std::ostream& os);

    void toJson(std::ostream& os) const { toJson(httpStatus(), httpReason(), what(), os); }

    virtual int httpStatus() const noexcept = 0;

    virtual const char* httpReason() const noexcept = 0;
};

inline std::ostream& operator<<(std::ostream& os, const ServException& e)
{
    e.toJson(os);
    return os;
}

/**
 * The request could not be understood by the server due to malformed syntax. The client SHOULD NOT
 * repeat the request without modifications.
 */
class MVS_API BadRequestException : public ServException {
public:
    explicit BadRequestException(string_view what) noexcept : ServException{what} {}
    ~BadRequestException() noexcept override;

    // Copy.
    BadRequestException(const BadRequestException&) noexcept = default;
    BadRequestException& operator=(const BadRequestException&) noexcept = default;

    // Move.
    BadRequestException(BadRequestException&&) noexcept = default;
    BadRequestException& operator=(BadRequestException&&) noexcept = default;

    int httpStatus() const noexcept override;

    const char* httpReason() const noexcept override;
};

class MVS_API AlreadyExistsException : public BadRequestException {
public:
    explicit AlreadyExistsException(string_view what) noexcept : BadRequestException{what} {}
    ~AlreadyExistsException() noexcept override;

    // Copy.
    AlreadyExistsException(const AlreadyExistsException&) noexcept = default;
    AlreadyExistsException& operator=(const AlreadyExistsException&) noexcept = default;

    // Move.
    AlreadyExistsException(AlreadyExistsException&&) noexcept = default;
    AlreadyExistsException& operator=(AlreadyExistsException&&) noexcept = default;
};

class MVS_API RefAlreadyExistsException : public AlreadyExistsException {
public:
    explicit RefAlreadyExistsException(string_view what) noexcept : AlreadyExistsException{what}
    {
    }
    RefAlreadyExistsException() noexcept = default;

    ~RefAlreadyExistsException() noexcept override;

    // Copy.
    RefAlreadyExistsException(const RefAlreadyExistsException&) noexcept = default;
    RefAlreadyExistsException& operator=(const RefAlreadyExistsException&) noexcept = default;

    // Move.
    RefAlreadyExistsException(RefAlreadyExistsException&&) noexcept = default;
    RefAlreadyExistsException& operator=(RefAlreadyExistsException&&) noexcept = default;
};

class MVS_API InvalidException : public BadRequestException {
public:
    explicit InvalidException(string_view what) noexcept : BadRequestException{what} {}
    ~InvalidException() noexcept override;

    // Copy.
    InvalidException(const InvalidException&) noexcept = default;
    InvalidException& operator=(const InvalidException&) noexcept = default;

    // Move.
    InvalidException(InvalidException&&) noexcept = default;
    InvalidException& operator=(InvalidException&&) noexcept = default;
};

/**
 * The server understood the request, but is refusing to fulfill it. Authorization will not help and
 * the request SHOULD NOT be repeated. If the request method was not HEAD and the server wishes to
 * make public why the request has not been fulfilled, it SHOULD describe the reason for the refusal
 * in the entity. If the server does not wish to make this information available to the client, the
 * status code 404 (Not Found) can be used instead.
 */
class MVS_API ForbiddenException : public ServException {
public:
    explicit ForbiddenException(string_view what) noexcept : ServException{what} {}
    ~ForbiddenException() noexcept override;

    // Copy.
    ForbiddenException(const ForbiddenException&) noexcept = default;
    ForbiddenException& operator=(const ForbiddenException&) noexcept = default;

    // Move.
    ForbiddenException(ForbiddenException&&) noexcept = default;
    ForbiddenException& operator=(ForbiddenException&&) noexcept = default;

    int httpStatus() const noexcept override;

    const char* httpReason() const noexcept override;
};

/**
 * The server encountered an unexpected condition which prevented it from fulfilling the request.
 */
class MVS_API InternalException : public ServException {
public:
    explicit InternalException(string_view what) noexcept : ServException{what} {}
    ~InternalException() noexcept override;

    // Copy.
    InternalException(const InternalException&) noexcept = default;
    InternalException& operator=(const InternalException&) noexcept = default;

    // Move.
    InternalException(InternalException&&) noexcept = default;
    InternalException& operator=(InternalException&&) noexcept = default;

    int httpStatus() const noexcept override;

    const char* httpReason() const noexcept override;
};

/**
 * The method specified in the Request-Line is not allowed for the resource identified by the
 * Request-URI. The response MUST include an Allow header containing a list of valid methods for the
 * requested resource.
 */
class MVS_API MethodNotAllowedException : public ServException {
public:
    explicit MethodNotAllowedException(string_view what) noexcept : ServException{what} {}
    ~MethodNotAllowedException() noexcept override;

    // Copy.
    MethodNotAllowedException(const MethodNotAllowedException&) noexcept = default;
    MethodNotAllowedException& operator=(const MethodNotAllowedException&) noexcept = default;

    // Move.
    MethodNotAllowedException(MethodNotAllowedException&&) noexcept = default;
    MethodNotAllowedException& operator=(MethodNotAllowedException&&) noexcept = default;

    int httpStatus() const noexcept override;

    const char* httpReason() const noexcept override;
};

/**
 * The server has not found anything matching the Request-URI. No indication is given of whether the
 * condition is temporary or permanent. The 410 (Gone) status code SHOULD be used if the server
 * knows, through some internally configurable mechanism, that an old resource is permanently
 * unavailable and has no forwarding address. This status code is commonly used when the server does
 * not wish to reveal exactly why the request has been refused, or when no other response is
 * applicable.
 */
class MVS_API NotFoundException : public ServException {
public:
    explicit NotFoundException(string_view what) noexcept : ServException{what} {}
    ~NotFoundException() noexcept override;

    // Copy.
    NotFoundException(const NotFoundException&) noexcept = default;
    NotFoundException& operator=(const NotFoundException&) noexcept = default;

    // Move.
    NotFoundException(NotFoundException&&) noexcept = default;
    NotFoundException& operator=(NotFoundException&&) noexcept = default;

    int httpStatus() const noexcept override;

    const char* httpReason() const noexcept override;
};

/**
 * The server is currently unable to handle the request due to a temporary overloading or
 * maintenance of the server. The implication is that this is a temporary condition which will be
 * alleviated after some delay. If known, the length of the delay MAY be indicated in a Retry-After
 * header. If no Retry-After is given, the client SHOULD handle the response as it would for a 500
 * response.
 */
class MVS_API ServiceUnavailableException : public ServException {
public:
    explicit ServiceUnavailableException(string_view what) noexcept : ServException{what} {}
    ~ServiceUnavailableException() noexcept override;

    // Copy.
    ServiceUnavailableException(const ServiceUnavailableException&) noexcept = default;
    ServiceUnavailableException& operator=(const ServiceUnavailableException&) noexcept = default;

    // Move.
    ServiceUnavailableException(ServiceUnavailableException&&) noexcept = default;
    ServiceUnavailableException& operator=(ServiceUnavailableException&&) noexcept = default;

    int httpStatus() const noexcept override;

    const char* httpReason() const noexcept override;
};

/**
 * The request requires user authentication. The response MUST include a WWW-Authenticate header
 * field (section 14.47) containing a challenge applicable to the requested resource. The client MAY
 * repeat the request with a suitable Authorization header field (section 14.8). If the request
 * already included Authorization credentials, then the 401 response indicates that authorization
 * has been refused for those credentials. If the 401 response contains the same challenge as the
 * prior response, and the user agent has already attempted authentication at least once, then the
 * user SHOULD be presented the entity that was given in the response, since that entity might
 * include relevant diagnostic information. HTTP access authentication is explained in "HTTP
 * Authentication: Basic and Digest Access Authentication".
 */
class MVS_API UnauthorizedException : public ServException {
public:
    explicit UnauthorizedException(string_view what) noexcept : ServException{what} {}
    ~UnauthorizedException() noexcept override;

    // Copy.
    UnauthorizedException(const UnauthorizedException&) noexcept = default;
    UnauthorizedException& operator=(const UnauthorizedException&) noexcept = default;

    // Move.
    UnauthorizedException(UnauthorizedException&&) noexcept = default;
    UnauthorizedException& operator=(UnauthorizedException&&) noexcept = default;

    int httpStatus() const noexcept override;

    const char* httpReason() const noexcept override;
};

} // mgbubble

/** @} */

