/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
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
#ifndef LIBBITCOIN_BLOCKCHAIN_BLOCK_FETCHER_HPP
#define LIBBITCOIN_BLOCKCHAIN_BLOCK_FETCHER_HPP

#include <cstdint>
#include <memory>
#include <system_error>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/blockchain/define.hpp>
#include <bitcoin/blockchain/block_chain.hpp>

namespace libbitcoin {
namespace blockchain {

/**
 * Fetch a block by height.
 *
 * If the blockchain reorganises this call may fail.
 *
 * @param[in]   chain           Blockchain service
 * @param[in]   height          Height of block to fetch.
 * @param[in]   handle_fetch    Completion handler for fetch operation.
 */
BCB_API void fetch_block(block_chain& chain, uint64_t height,
    block_chain::block_fetch_handler handle_fetch);

/**
 * Fetch a block by hash.
 *
 * If the blockchain reorganises this call may fail.
 *
 * @param[in]   chain           Blockchain service
 * @param[in]   hash            Block hash
 * @param[in]   handle_fetch    Completion handler for fetch operation.
 */
BCB_API void fetch_block(block_chain& chain, const hash_digest& hash,
    block_chain::block_fetch_handler handle_fetch);

} // namespace blockchain
} // namespace libbitcoin

#endif

