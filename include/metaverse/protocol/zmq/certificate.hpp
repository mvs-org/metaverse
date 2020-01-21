/**
 * Copyright (c) 2019-2020 libbitcoin developers (see AUTHORS)
 *
 * This file is part of metaverse-protocol.
 *
 * metaverse-protocol is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MVS_PROTOCOL_ZMQ_CERTIFICATE_HPP
#define MVS_PROTOCOL_ZMQ_CERTIFICATE_HPP

#include <string>
#include <metaverse/bitcoin.hpp>
#include <metaverse/protocol/define.hpp>

namespace libbitcoin {
namespace protocol {
namespace zmq {

/// This class is not thread safe.
/// A simplified "certificate" class to manage a curve key pair.
/// If valid the class always retains a consistent key pair.
class BCP_API certificate
{
public:
    /// Construct an arbitary keypair as a new certificate.
    /// This always reduces keyspace, disallowing '#' in text encoding.
    /// Use certificate({ null_hash }) to allow full key space.
    certificate();

    /// Construct a certificate from private key (generates public key).
    /// This generates an arbitary key pair if the parameter is uninitialized.
    certificate(const config::sodium& private_key);

    /// True if the certificate is valid.
    operator const bool() const;

    /// The public key base85 text.
    const config::sodium& public_key() const;

    /// The private key base85 text.
    const config::sodium& private_key() const;

protected:
    static bool derive(config::sodium& out_public,
        const config::sodium& private_key);
    static bool create(config::sodium& out_public,
        config::sodium& out_private, bool setting);

private:
    config::sodium public_;
    config::sodium private_;
};

} // namespace zmq
} // namespace protocol
} // namespace libbitcoin

#endif
