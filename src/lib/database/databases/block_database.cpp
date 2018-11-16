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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include <metaverse/database/databases/block_database.hpp>

#include <cstdint>
#include <cstddef>
#include <memory>
#include <boost/filesystem.hpp>
#include <metaverse/bitcoin.hpp>
#include <metaverse/database/memory/memory.hpp>
#include <metaverse/database/result/block_result.hpp>

namespace libbitcoin {
namespace database {

using namespace boost::filesystem;
using namespace bc::chain;

BC_CONSTEXPR size_t number_buckets = 600000;
BC_CONSTEXPR size_t header_size = slab_hash_table_header_size(number_buckets);
BC_CONSTEXPR size_t initial_map_file_size = header_size + minimum_slabs_size;

// Valid file offsets should never be zero.
const file_offset block_database::empty = 0;

// Record format:
// main:
//  [ header:80      ]
//  [ height:4       ]
//  [ number_txs:4   ]
// hashes:
//  [ [    ...     ] ]
//  [ [ tx_hash:32 ] ]
//  [ [    ...     ] ]

block_database::block_database(const path& map_filename,
    const path& index_filename, std::shared_ptr<shared_mutex> mutex)
  : lookup_file_(map_filename, mutex),
    lookup_header_(lookup_file_, number_buckets),
    lookup_manager_(lookup_file_, header_size),
    lookup_map_(lookup_header_, lookup_manager_),
    index_file_(index_filename, mutex),
    index_manager_(index_file_, 0, sizeof(file_offset))
{
}

// Close does not call stop because there is no way to detect thread join.
block_database::~block_database()
{
    close();
}

// Create.
// ----------------------------------------------------------------------------

// Initialize files and start.
bool block_database::create()
{
    // Resize and create require a started file.
    if (!lookup_file_.start() ||
        !index_file_.start())
        return false;

    // These will throw if insufficient disk space.
    lookup_file_.resize(initial_map_file_size);
    index_file_.resize(minimum_records_size);

    if (!lookup_header_.create() ||
        !lookup_manager_.create() ||
        !index_manager_.create())
        return false;

    // Should not call start after create, already started.
    return
        lookup_header_.start() &&
        lookup_manager_.start() &&
        index_manager_.start();
}

// Startup and shutdown.
// ----------------------------------------------------------------------------

// Start files and primitives.
bool block_database::start()
{
    return
        lookup_file_.start() &&
        index_file_.start() &&
        lookup_header_.start() &&
        lookup_manager_.start() &&
        index_manager_.start();
}

// Stop files.
bool block_database::stop()
{
    return
        lookup_file_.stop() &&
        index_file_.stop();
}

// Close files.
bool block_database::close()
{
    return
        lookup_file_.close() &&
        index_file_.close();
}

// ----------------------------------------------------------------------------

block_result block_database::get(size_t height) const
{
    if (height >= index_manager_.count())
        return block_result(nullptr);

    const auto position = read_position(height);
    const auto memory = lookup_manager_.get(position);
    return block_result(memory);
}

block_result block_database::get(const hash_digest& hash) const
{
    const auto memory = lookup_map_.find(hash);
    return block_result(memory);
}

void block_database::store(const block& block)
{
    store(block, index_manager_.count());
}

void block_database::store(const block& block, size_t height)
{
    BITCOIN_ASSERT(height <= max_uint32);
    const auto height32 = static_cast<uint32_t>(height);
    const auto tx_count = block.transactions.size();

    BITCOIN_ASSERT(tx_count <= max_uint32);
    const auto tx_count32 = static_cast<uint32_t>(tx_count);

    // Write block data.
    const auto write = [&](memory_ptr data)
    {
        auto serial = make_serializer(REMAP_ADDRESS(data));
        const auto header_data = block.header.to_data(false);
        serial.write_data(header_data);
        serial.write_4_bytes_little_endian(height32);
        serial.write_4_bytes_little_endian(tx_count32);

        for (const auto& tx: block.transactions)
            serial.write_hash(tx.hash());

        if (block.header.version == 2){
            //data_chunk sig;
            //std::copy(block.blocksig.begin(), block.blocksig.end(), sig.begin());
            serial.write_data(block.blocksig);
        }
    };

    const auto key = block.header.hash();
    const auto value_size = 148 + 4 + 4 + tx_count * hash_size;

    // Write block header, height, tx count and hashes to hash table.
    const auto position = lookup_map_.store(key, write, value_size);

    // Write block height to hash table position mapping to block index.
    write_position(position, height32);
}

void block_database::unlink(size_t from_height)
{
    if (index_manager_.count() > from_height)
        index_manager_.set_count(from_height);
}
void block_database::remove(const hash_digest& hash)
{
    DEBUG_ONLY(bool success =) lookup_map_.unlink(hash);
    BITCOIN_ASSERT(success);
}

void block_database::sync()
{
    lookup_manager_.sync();
    index_manager_.sync();
}

// This is necessary for parallel import, as gaps are created.
void block_database::zeroize(array_index first, array_index count)
{
    for (auto index = first; index < (first + count); ++index)
    {
        const auto memory = index_manager_.get(index);
        auto serial = make_serializer(REMAP_ADDRESS(memory));
        serial.write_8_bytes_little_endian(empty);
    }
}

void block_database::write_position(file_offset position, array_index height)
{
    BITCOIN_ASSERT(height < max_uint32);
    const auto new_count = height + 1;

    // Critical Section
    ///////////////////////////////////////////////////////////////////////////
    mutex_.lock_upgrade();

    // Guard index_manager to prevent interim count increase.
    const auto initial_count = index_manager_.count();

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    mutex_.unlock_upgrade_and_lock();

    // Guard write to prevent overwriting preceding height write.
    if (new_count > initial_count)
    {
        const auto create_count = new_count - initial_count;
        index_manager_.new_records(create_count);
        zeroize(initial_count, create_count - 1);
    }

    // Guard write to prevent subsequent zeroize from erasing.
    const auto memory = index_manager_.get(height);
    auto serial = make_serializer(REMAP_ADDRESS(memory));
    serial.write_8_bytes_little_endian(position);

    mutex_.unlock();
    ///////////////////////////////////////////////////////////////////////////
}

file_offset block_database::read_position(array_index height) const
{
    const auto memory = index_manager_.get(height);
    const auto address = REMAP_ADDRESS(memory);
    return from_little_endian_unsafe<file_offset>(address);
}

// The index of the highest existing block, independent of gaps.
bool block_database::top(size_t& out_height) const
{
    const auto count = index_manager_.count();

    // Guard against no genesis block.
    if (count == 0)
        return false;

    out_height = count - 1;
    return true;
}

bool block_database::gap_range(size_t& out_first, size_t& out_last) const
{
    size_t first;
    const auto count = index_manager_.count();

    for (first = 0; first < count; ++first)
    {
        if (read_position(first) == empty)
        {
            // There is at least one gap.
            out_first = first;
            break;
        }
    }

    // There are no gaps.
    if (first == count)
        return false;

    for (size_t last = count - 1; last > first; --last)
    {
        if (read_position(last) == empty)
        {
            // There are at least two gaps.
            out_last = last;
            return true;
        }
    }

    // There is only one gap.
    out_last = first;
    return true;
}

bool block_database::next_gap(size_t& out_height, size_t start_height) const
{
    const auto count = index_manager_.count();

    // Guard against no genesis block, terminate is starting after last gap.
    if (count == 0 || start_height > count)
        return false;

    // Scan for first missing block and return its parent block height.
    for (size_t height = start_height; height < count; ++height)
    {
        if (read_position(height) == empty)
        {
            out_height = height;
            return true;
        }
    }

    // There are no gaps in the chain, count is the last gap.
    out_height = count;
    return true;
}

} // namespace database
} // namespace libbitcoin
