/**
 * Copyright (c) 2011-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2020 metaverse core developers (see MVS-AUTHORS)
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
#ifndef MVS_WALLET_HD_PRIVATE_KEY_HPP
#define MVS_WALLET_HD_PRIVATE_KEY_HPP

#include <cstdint>
#include <string>
#include <metaverse/bitcoin/define.hpp>
#include <metaverse/bitcoin/math/elliptic_curve.hpp>
#include <metaverse/bitcoin/utility/data.hpp>
#include <metaverse/bitcoin/wallet/ec_private.hpp>
#include <metaverse/bitcoin/wallet/ec_public.hpp>
#include <metaverse/bitcoin/wallet/hd_public.hpp>

namespace libbitcoin {
namespace wallet {

/// An extended private key, as defined by BIP 32.
class BC_API hd_private
  : public hd_public
{
public:
    static const uint64_t mainnet;

    static inline uint32_t to_prefix(uint64_t prefixes)
    {
        return prefixes >> 32;
    }

    static inline uint64_t to_prefixes(uint32_t private_prefix,
        uint32_t public_prefix)
    {
        return uint64_t(private_prefix) << 32 | public_prefix;
    }

    /// Constructors.
    hd_private();
    hd_private(const hd_private& other);
    hd_private(const data_chunk& seed, uint64_t prefixes=mainnet);
    hd_private(const hd_key& private_key);
    hd_private(const hd_key& private_key, uint64_t prefixes);
    hd_private(const hd_key& private_key, uint32_t public_prefix);
    hd_private(const std::string& encoded);
    hd_private(const std::string& encoded, uint64_t prefixes);
    hd_private(const std::string& encoded, uint32_t public_prefix);

    /// Operators.
    bool operator<(const hd_private& other) const;
    bool operator==(const hd_private& other) const;
    bool operator!=(const hd_private& other) const;
    hd_private& operator=(const hd_private& other);
    friend std::istream& operator>>(std::istream& in, hd_private& to);
    friend std::ostream& operator<<(std::ostream& out,
        const hd_private& of);

    /// Cast operators.
    operator const ec_secret&() const;

    /// Serializer.
    std::string encoded() const;

    /// Accessors.
    const ec_secret& secret() const;

    /// Methods.
    hd_key to_hd_key() const;
    hd_public to_public() const;
    hd_private derive_private(uint32_t index) const;
    hd_public derive_public(uint32_t index) const;

private:
    /// Factories.
    static hd_private from_seed(data_slice seed, uint64_t prefixes);
    static hd_private from_key(const hd_key& decoded);
    static hd_private from_key(const hd_key& decoded, uint32_t prefix);
    static hd_private from_key(const hd_key& decoded, uint64_t public_prefix);
    static hd_private from_string(const std::string& encoded);
    static hd_private from_string(const std::string& encoded,
        uint32_t public_prefix);
    static hd_private from_string(const std::string& encoded,
        uint64_t prefixes);

    hd_private(const ec_secret& secret, const hd_chain_code& chain_code,
        const hd_lineage& lineage);

    /// Members.
    /// This should be const, apart from the need to implement assignment.
    ec_secret secret_;
};

} // namespace wallet
} // namespace libbitcoin

#endif
