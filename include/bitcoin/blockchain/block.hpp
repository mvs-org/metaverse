/*
 * Copyright (c) 2011-2013 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
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
#ifndef LIBBITCOIN_BLOCKCHAIN_BLOCK_LOCATOR_INDEXES_HPP
#define LIBBITCOIN_BLOCKCHAIN_BLOCK_LOCATOR_INDEXES_HPP

#include <cstdint>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/blockchain/define.hpp>

namespace libbitcoin {
namespace blockchain {

BCB_API uint64_t block_subsidy(size_t height);

BCB_API hash_number block_work(uint32_t bits);

BCB_API chain::block::indexes block_locator_indexes(size_t top_height);

} // namespace blockchain
} // namespace libbitcoin

#endif
