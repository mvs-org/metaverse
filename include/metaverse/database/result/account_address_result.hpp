/**
 * Copyright (c) 2011-2020 metaverse developers (see AUTHORS)
 *
 * This file is part of mvs-node.
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
#ifndef MVS_DATABASE_ACCOUNT_ADDRESS_RESULT_HPP
#define MVS_DATABASE_ACCOUNT_ADDRESS_RESULT_HPP

#include <cstddef>
#include <cstdint>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/define.hpp>
#include <metaverse/database/memory/memory.hpp>
#include <metaverse/database/result/base_result.hpp>
#include <metaverse/bitcoin/chain/attachment/account/account_address.hpp>

namespace libbitcoin {
namespace database {

/// read account_address detail information from account_address database.
class BCD_API account_address_result : public base_result
{
public:
    account_address_result(const memory_ptr slab);

    /// The account_address.
    std::shared_ptr<chain::account_address> get_account_address_detail() const;
};

} // namespace database
} // namespace libbitcoin

#endif
