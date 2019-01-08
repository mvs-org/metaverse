/**
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/blockchain/profile.hpp>
#include <metaverse/blockchain/block_chain_impl.hpp>

namespace libbitcoin {
namespace blockchain {

bool profile::check_context(const profile_context& context)
{
    // check type
    auto& type = context.type;
    switch (type) {
        case profile_type::witness:
            break;
        case profile_type::none:
        default:
            return false;
    }
    // check height range
    auto& range = context.height_range;
    if (range.first >= range.second) {
        return false;
    }
    // check did
    auto& chain = context.block_chain;
    auto& did = context.did;
    if (!chain.is_did_exist(did)) {
        return false;
    }
    return true;
}

witness_profile::witness_profile()
    : profile()
{
}

witness_profile::~witness_profile()
{
}

profile::ptr witness_profile::get_profile(const profile_context& context) const
{
    if (!check_context(context)) {
        return nullptr;
    }
    return std::make_shared<witness_profile>(*this);
}

} // namespace blockchain
} // namespace libbitcoin
