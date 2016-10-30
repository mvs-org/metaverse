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
#ifndef LIBBITCOIN_BLOCKCHAIN_IMPL_VALIDATE_BLOCK_H
#define LIBBITCOIN_BLOCKCHAIN_IMPL_VALIDATE_BLOCK_H

#include <cstddef>
#include <cstdint>
#include <vector>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/blockchain/simple_chain.hpp>
#include <bitcoin/blockchain/validate_block.hpp>

namespace libbitcoin {
namespace blockchain {

/// This class is not thread safe.
class BCB_API validate_block_impl
  : public validate_block
{
public:
    validate_block_impl(simple_chain& chain, size_t fork_index,
        const block_detail::list& orphan_chain, size_t orphan_index,
        size_t height, const chain::block& block, bool testnet,
        const config::checkpoint::list& checkpoints,
        stopped_callback stopped);

protected:
    uint64_t median_time_past() const;
    uint32_t previous_block_bits() const;
    uint64_t actual_time_span(size_t interval) const;
    versions preceding_block_versions(size_t maximum) const;
    chain::header fetch_block(size_t fetch_height) const;
    bool fetch_transaction(chain::transaction& tx, size_t& tx_height,
        const hash_digest& tx_hash) const;
    bool is_output_spent(const chain::output_point& outpoint) const;
    bool is_output_spent(const chain::output_point& previous_output,
        size_t index_in_parent, size_t input_index) const;
    bool transaction_exists(const hash_digest& tx_hash) const;

private:
    bool fetch_orphan_transaction(chain::transaction& tx,
        size_t& previous_height, const hash_digest& tx_hash) const;
    bool orphan_is_spent(const chain::output_point& previous_output,
        size_t skip_tx, size_t skip_input) const;

    simple_chain& chain_;
    size_t height_;
    size_t fork_index_;
    size_t orphan_index_;
    const block_detail::list& orphan_chain_;
};

} // namespace blockchain
} // namespace libbitcoin

#endif
