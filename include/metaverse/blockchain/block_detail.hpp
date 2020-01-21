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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MVS_BLOCKCHAIN_BLOCK_DETAIL_HPP
#define MVS_BLOCKCHAIN_BLOCK_DETAIL_HPP

#include <atomic>
#include <memory>
#include <vector>
#include <metaverse/bitcoin.hpp>
#include <metaverse/blockchain/define.hpp>

namespace libbitcoin {
namespace blockchain {

/// A block with metadata.
/// This class is thread safe though property consistency is not guaranteed.
class BCB_API block_detail
{
public:
    typedef std::shared_ptr<block_detail> ptr;
    typedef std::vector<block_detail::ptr> list;
    typedef message::block_message::ptr block_ptr;

    /// Construct a block detail instance.
    block_detail(block_ptr actual_block);
    block_detail(chain::block&& actual_block);

    block_ptr actual() const;

    /// Set a flag indicating validation has been completed.
    void set_processed();
    bool processed() const;

    /// Set the accepted block height (non-zero).
    void set_height(uint64_t height);
    uint64_t height() const;

    /// Set the validation failure code.
    void set_error(const code& code);
    code error() const;

    /// This method is thread safe.
    //chenhao remove & from hash_digest
    const hash_digest hash() const;

    // Set if work proof is checked.
    void set_is_checked_work_proof(bool is_checked);
    bool get_is_checked_work_proof() const;

private:
    bc::atomic<code> code_;
    std::atomic<bool> processed_;
    std::atomic<uint64_t> height_;
    const block_ptr actual_block_;
    std::atomic<bool> is_checked_work_proof_;
};

} // namespace blockchain
} // namespace libbitcoin

#endif
