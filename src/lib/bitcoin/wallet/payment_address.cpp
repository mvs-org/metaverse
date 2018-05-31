/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2018 metaverse core developers (see MVS-AUTHORS)
 *
 * This file is part of metaverse.
 *
 * metaverse is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/bitcoin/wallet/payment_address.hpp>

#include <algorithm>
#include <cstdint>
#include <string>
#include <boost/program_options.hpp>
#include <metaverse/bitcoin/formats/base_58.hpp>
#include <metaverse/bitcoin/math/checksum.hpp>
#include <metaverse/bitcoin/math/hash.hpp>
#include <metaverse/bitcoin/wallet/ec_private.hpp>
#include <metaverse/bitcoin/wallet/ec_public.hpp>

namespace libbitcoin {
namespace wallet {

//chenhao bad modify
uint8_t payment_address::mainnet_p2kh = 0x32;
const uint8_t payment_address::mainnet_p2sh = 0x05;
const std::string payment_address::blackhole_address = "1111111111111111111114oLvT2";

payment_address::payment_address()
  : valid_(false), version_(0), hash_(null_short_hash)
{
}

payment_address::payment_address(const payment_address& other)
  : valid_(other.valid_), version_(other.version_), hash_(other.hash_)
{
}

payment_address::payment_address(const payment& decoded)
  : payment_address(from_payment(decoded))
{
}

payment_address::payment_address(const std::string& address)
  : payment_address(from_string(address))
{
}

// MSVC (CTP) casts this down to 8 bits if a variable is not used:
// const payment_address address({ ec_secret, 0x806F });
// Alternatively explicit construction of the ec_private also works.
// const payment_address address(ec_private(ec_secret, 0x806F));
// const payment_address address(ec_private{ ec_secret, 0x806F });
// However this doesn't impact payment_address, since it only uses the LSB.
payment_address::payment_address(const ec_private& secret)
  : payment_address(from_private(secret))
{
}

payment_address::payment_address(const ec_public& point, uint8_t version)
  : payment_address(from_public(point, version))
{
}

payment_address::payment_address(const chain::script& script, uint8_t version)
  : payment_address(from_script(script, version))
{
}

payment_address::payment_address(const short_hash& hash, uint8_t version)
  : valid_(true), version_(version), hash_(hash)
{
}

// Validators.
// ----------------------------------------------------------------------------

bool payment_address::is_address(data_slice decoded)
{
    return (decoded.size() == payment_size) && verify_checksum(decoded);
}

// Factories.
// ----------------------------------------------------------------------------

payment_address payment_address::from_string(const std::string& address)
{
    payment decoded;
    if (!decode_base58(decoded, address) || !is_address(decoded))
        return payment_address();

    return payment_address(decoded);
}

payment_address payment_address::from_payment(const payment& decoded)
{
    if (!is_address(decoded))
        return payment_address();

    const auto hash = slice<1, short_hash_size + 1>(decoded);
    return payment_address(hash, decoded.front());
}

payment_address payment_address::from_private(const ec_private& secret)
{
    if (!secret)
        return payment_address();

    return payment_address(secret.to_public(), secret.payment_version());
}

payment_address payment_address::from_public(const ec_public& point,
    uint8_t version)
{
    if (!point)
        return payment_address();

    data_chunk data;
    return point.to_data(data) ?
        payment_address(bitcoin_short_hash(data), version) :
        payment_address();
}

payment_address payment_address::from_script(const chain::script& script,
    uint8_t version)
{
    return payment_address(bitcoin_short_hash(script.to_data(false)), version);
}

// Cast operators.
// ----------------------------------------------------------------------------

payment_address::operator const bool() const
{
    return valid_;
}

payment_address::operator const short_hash&() const
{
    return hash_;
}

// Serializer.
// ----------------------------------------------------------------------------

std::string payment_address::encoded() const
{
    return encode_base58(wrap(version_, hash_));
}

// Accessors.
// ----------------------------------------------------------------------------

uint8_t payment_address::version() const
{
    return version_;
}

const short_hash& payment_address::hash() const
{
    return hash_;
}

// Methods.
// ----------------------------------------------------------------------------

payment payment_address::to_payment() const
{
    return wrap(version_, hash_);
}

// Operators.
// ----------------------------------------------------------------------------

payment_address& payment_address::operator=(const payment_address& other)
{
    valid_ = other.valid_;
    version_ = other.version_;
    hash_ = other.hash_;
    return *this;
}

bool payment_address::operator<(const payment_address& other) const
{
    return encoded() < other.encoded();
}

bool payment_address::operator==(const payment_address& other) const
{
    return valid_ == other.valid_ && version_ == other.version_ &&
        hash_ == other.hash_;
}

bool payment_address::operator!=(const payment_address& other) const
{
    return !(*this == other);
}

std::istream& operator>>(std::istream& in, payment_address& to)
{
    std::string value;
    in >> value;
    to = payment_address(value);

    if (!to)
    {
        using namespace boost::program_options;
        BOOST_THROW_EXCEPTION(invalid_option_value(value));
    }

    return in;
}

std::ostream& operator<<(std::ostream& out, const payment_address& of)
{
    out << of.encoded();
    return out;
}

// Static functions.
// ----------------------------------------------------------------------------

payment_address payment_address::extract(const chain::script& script,
    uint8_t p2kh_version, uint8_t p2sh_version)
{
    if (!script.is_valid())
        return payment_address();

    short_hash hash;
    const auto& ops = script.operations;

    // Split out the assertions for readability.
    // We know that the script is valid and can therefore rely on these.
    switch (script.pattern())
    {
        // pay
        // --------------------------------------------------------------------
        case chain::script_pattern::pay_multisig:
            break;
        case chain::script_pattern::pay_public_key:
            BITCOIN_ASSERT(ops.size() == 2);
            BITCOIN_ASSERT(
                ops[0].data.size() == ec_compressed_size ||
                ops[0].data.size() == ec_uncompressed_size);
            break;
        case chain::script_pattern::pay_key_hash:
            BITCOIN_ASSERT(ops.size() == 5);
            BITCOIN_ASSERT(ops[2].data.size() == short_hash_size);
            break;
        case chain::script_pattern::pay_key_hash_with_lock_height:
            BITCOIN_ASSERT(ops.size() == 7);
            BITCOIN_ASSERT(ops[4].data.size() == short_hash_size);
            break;
        case chain::script_pattern::pay_script_hash:
            BITCOIN_ASSERT(ops.size() == 3);
            BITCOIN_ASSERT(ops[1].data.size() == short_hash_size);
            break;
        case chain::script_pattern::pay_blackhole_address:
            BITCOIN_ASSERT(ops.size() == 1);
            break;

        // sign
        // --------------------------------------------------------------------
        case chain::script_pattern::sign_multisig:
            break;
        case chain::script_pattern::sign_public_key:
            break;
        case chain::script_pattern::sign_key_hash:
            BITCOIN_ASSERT(ops.size() == 2);
            BITCOIN_ASSERT(
                ops[1].data.size() == ec_compressed_size ||
                ops[1].data.size() == ec_uncompressed_size);
            break;
        case chain::script_pattern::sign_key_hash_with_lock_height:
            BITCOIN_ASSERT(ops.size() == 3);
            BITCOIN_ASSERT(
                ops[1].data.size() == ec_compressed_size ||
                ops[1].data.size() == ec_uncompressed_size);
            break;
        case chain::script_pattern::sign_script_hash:
            BITCOIN_ASSERT(ops.size() > 1);
            break;
        case chain::script_pattern::pay_key_hash_with_attenuation_model:
            BITCOIN_ASSERT(ops.size() == 8);
            BITCOIN_ASSERT(ops[5].data.size() == short_hash_size);
            break;
        case chain::script_pattern::non_standard:
        default:;
    }

    // Convert data to hash or point and construct address.
    switch (script.pattern())
    {
        // pay
        // --------------------------------------------------------------------

        case chain::script_pattern::pay_multisig:
            return payment_address();

        case chain::script_pattern::pay_public_key:
        {
            const auto& data = ops[0].data;
            if (data.size() == ec_compressed_size)
            {
                const auto point = to_array<ec_compressed_size>(data);
                return payment_address(point, p2kh_version);
            }

            const auto point = to_array<ec_uncompressed_size>(data);
            return payment_address(point, p2kh_version);
        }

        case chain::script_pattern::pay_key_hash:
            hash = to_array<short_hash_size>(ops[2].data);
            return payment_address(hash, p2kh_version);

        case chain::script_pattern::pay_key_hash_with_lock_height:
            hash = to_array<short_hash_size>(ops[4].data);
            return payment_address(hash, p2kh_version);

        case chain::script_pattern::pay_script_hash:
            hash = to_array<short_hash_size>(ops[1].data);
            return payment_address(hash, p2sh_version);

        case chain::script_pattern::pay_blackhole_address:
            return payment_address(blackhole_address);

        case chain::script_pattern::pay_key_hash_with_attenuation_model:
            hash = to_array<short_hash_size>(ops[5].data);
            return payment_address(hash, p2kh_version);

        // sign
        // --------------------------------------------------------------------

        case chain::script_pattern::sign_multisig:
        {
            bc::chain::script redeem_script;
            // extract address from multisig payment script
            // zero sig1 sig2 ... encoded-multisig
            const auto& redeem_data = ops.back().data;
            
            if (redeem_data.empty())
                return payment_address(); //throw std::logic_error{"empty redeem script."};
            
            if (!redeem_script.from_data(redeem_data, false, bc::chain::script::parse_mode::strict))
                return payment_address(); //throw std::logic_error{"error occured when parse redeem script data."};
            
            // Is the redeem script a standard pay (output) script?
            const auto redeem_script_pattern = redeem_script.pattern();
            if(redeem_script_pattern != chain::script_pattern::pay_multisig)
                return payment_address(); //throw std::logic_error{"redeem script is not pay multisig pattern."};
            
            const payment_address address(redeem_script, 5);
            //auto addr_str = address.encoded(); // pay address
            //log::trace("input_addr")<<redeem_script.to_string(false);
            //log::trace("input_addr")<<addr_str;            
            return address;
        }

        case chain::script_pattern::sign_public_key:
            return payment_address();

        case chain::script_pattern::sign_key_hash:
        case chain::script_pattern::sign_key_hash_with_lock_height:
        {
            const auto& data = ops[1].data;
            if (data.size() == ec_compressed_size)
            {
                const auto point = to_array<ec_compressed_size>(data);
                return payment_address(point, p2kh_version);
            }

            const auto point = to_array<ec_uncompressed_size>(data);
            return payment_address(point, p2kh_version);
        }

        case chain::script_pattern::sign_script_hash:
            hash = bitcoin_short_hash(ops.back().data);
            return payment_address(hash, p2sh_version);

        case chain::script_pattern::non_standard:
        default:
            return payment_address();
    }
}

} // namespace wallet
} // namespace libbitcoin
