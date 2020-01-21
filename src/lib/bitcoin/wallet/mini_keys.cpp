/**
 * Copyright (c) 2019-2020 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2019-2020 metaverse core developers (see MVS-AUTHORS)
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
#include <metaverse/bitcoin/wallet/mini_keys.hpp>

#include <cstdint>
#include <string>
#include <metaverse/bitcoin/math/elliptic_curve.hpp>
#include <metaverse/bitcoin/math/hash.hpp>

namespace libbitcoin {
namespace wallet {

bool check_minikey(const std::string& minikey)
{
    // Legacy minikeys are 22 chars long
    bool valid = minikey.size() == 22 || minikey.size() == 30;
    return valid && sha256_hash(to_chunk(minikey + "?"))[0] == 0x00;
}

bool minikey_to_secret(ec_secret& out_secret, const std::string& key)
{
    if (!check_minikey(key))
        return false;

    out_secret = sha256_hash(to_chunk(key));
    return true;
}

} // namespace wallet
} // namespace libbitcoin
