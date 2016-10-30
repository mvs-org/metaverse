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
#include <bitcoin/blockchain/block_fetcher.hpp>

#include <atomic>
#include <cstdint>
#include <memory>
#include <system_error>
#include <bitcoin/blockchain/block_chain.hpp>

namespace libbitcoin {
namespace blockchain {

using namespace chain;
using namespace std::placeholders;

// TODO: split into header.
// This class is used only locally.
class block_fetcher
  : public std::enable_shared_from_this<block_fetcher>
{
public:
    block_fetcher(block_chain& chain)
      : blockchain_(chain)
    {
    }

    template <typename BlockIndex>
    void start(const BlockIndex& index,
        block_chain::block_fetch_handler handler)
    {
        // Create the block.
        const auto block = std::make_shared<chain::block>();

        blockchain_.fetch_block_header(index,
            std::bind(&block_fetcher::handle_fetch_header,
                shared_from_this(), _1, _2, block, handler));
    }

private:
    void handle_fetch_header(const code& ec, const header& header,
        block::ptr block, block_chain::block_fetch_handler handler)
    {
        if (ec)
        {
            handler(ec, nullptr);
            return;
        }

        // Set the block header.
        block->header = header;
        const auto hash = header.hash();

        blockchain_.fetch_block_transaction_hashes(hash,
            std::bind(&block_fetcher::fetch_transactions,
                shared_from_this(), _1, _2, block, handler));
    }

    void fetch_transactions(const code& ec, const hash_list& hashes,
        block::ptr block, block_chain::block_fetch_handler handler)
    {
        if (ec)
        {
            handler(ec, nullptr);
            return;
        }

        // Set the block transaction size.
        const auto count = hashes.size();
        block->transactions.resize(count);

        // This will be called exactly once by the synchronizer.
        const auto completion_handler =
            std::bind(&block_fetcher::handle_complete,
                shared_from_this(), _1, _2, handler);

        // Synchronize transaction fetch calls to one completion call.
        const auto complete = synchronize(completion_handler, count,
            "block_fetcher");

        // blockchain::fetch_transaction is thread safe.
        size_t index = 0;
        for (const auto& hash: hashes)
            blockchain_.fetch_transaction(hash,
                std::bind(&block_fetcher::handle_fetch_transaction,
                    shared_from_this(), _1, _2, index++, block, complete));
    }

    void handle_fetch_transaction(const code& ec,
        const transaction& transaction, size_t index, block::ptr block,
        block_chain::block_fetch_handler handler)
    {
        if (ec)
        {
            handler(ec, nullptr);
            return;
        }

        // Critical Section
        ///////////////////////////////////////////////////////////////////////
        mutex_.lock();

        // TRANSACTION COPY
        block->transactions[index] = transaction;

        mutex_.unlock();
        ///////////////////////////////////////////////////////////////////////

        handler(error::success, block);
    }

    // If ec success then there is no possibility that block is being written.
    void handle_complete(const code& ec, block::ptr block,
        block_chain::block_fetch_handler handler)
    {
        if (ec)
            handler(ec, nullptr);
        else
            handler(error::success, block);
    }

    block_chain& blockchain_;
    mutable shared_mutex mutex_;
};

void fetch_block(block_chain& chain, uint64_t height,
    block_chain::block_fetch_handler handle_fetch)
{
    const auto fetcher = std::make_shared<block_fetcher>(chain);
    fetcher->start(height, handle_fetch);
}

void fetch_block(block_chain& chain, const hash_digest& hash,
    block_chain::block_fetch_handler handle_fetch)
{
    const auto fetcher = std::make_shared<block_fetcher>(chain);
    fetcher->start(hash, handle_fetch);
}

} // namespace blockchain
} // namespace libbitcoin
