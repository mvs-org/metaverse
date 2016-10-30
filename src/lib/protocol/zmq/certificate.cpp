/**
 * Copyright (c) 2011-2016 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin-protocol.
 *
 * libbitcoin-protocol is free software: you can redistribute it and/or
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
#include <bitcoin/protocol/zmq/certificate.hpp>

#include <string>
#include <zmq.h>
#include <bitcoin/bitcoin.hpp>

namespace libbitcoin {
namespace protocol {
namespace zmq {

static constexpr int32_t zmq_fail = -1;
static constexpr size_t zmq_encoded_key_size = 40;

certificate::certificate()
{
    // HACK: restricted key space for use with config files.
    create(public_, private_, true);
}

// Full key space.
certificate::certificate(const config::sodium& private_key)
{
    if (!private_key)
    {
        // Full key space (may include '#' character in z85 encoding).
        create(public_, private_, false);
        return;
    }

    if (derive(public_, private_key))
        private_ = private_key;
}

bool certificate::derive(config::sodium& out_public,
    const config::sodium& private_key)
{
    if (!private_key)
        return false;

    const auto key = private_key.to_string();
    char public_key[zmq_encoded_key_size + 1] = { 0 };

    if (zmq_curve_public(public_key, key.data()) == zmq_fail)
        return false;

    out_public = config::sodium(public_key);
    return out_public;
}

// TODO: update settings loader so this isn't necessary.
// BUGBUG: this limitation weakens security by reducing key space.
static inline bool ok_setting(const std::string& key)
{
    return key.find_first_of('#') == std::string::npos;
};

bool certificate::create(config::sodium& out_public,
    config::sodium& out_private, bool setting)
{
    // Loop until neither key's base85 encoding includes the # character.
    // This ensures that the value can be used in libbitcoin settings files.
    for (uint8_t attempt = 0; attempt <= max_uint8; attempt++)
    {
        char public_key[zmq_encoded_key_size + 1] = { 0 };
        char private_key[zmq_encoded_key_size + 1] = { 0 };

        if (zmq_curve_keypair(public_key, private_key) == zmq_fail)
            return false;

        if (!setting || ((ok_setting(public_key) && ok_setting(private_key))))
        {
            out_public = config::sodium(public_key);
            out_private = config::sodium(private_key);
            return out_public;
        }
    }

    return false;
}

certificate::operator const bool() const
{
    return public_;
}

const config::sodium& certificate::public_key() const
{
    return public_;
}

const config::sodium& certificate::private_key() const
{
    return private_;
}

} // namespace zmq
} // namespace protocol
} // namespace libbitcoin
