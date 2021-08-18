/**
 * Copyright (c) 2011-2021 libbitcoin developers (see AUTHORS)
 * Copyright (c) 2016-2021 metaverse core developers (see MVS-AUTHORS)
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
#ifndef MVS_DATABASE_STEALTH_DATABASE_HPP
#define MVS_DATABASE_STEALTH_DATABASE_HPP

#include <cstdint>
#include <memory>
#include <boost/filesystem.hpp>
#include <metaverse/database/define.hpp>
#include <metaverse/database/memory/memory.hpp>
#include <metaverse/database/memory/memory_map.hpp>
#include <metaverse/database/primitives/record_manager.hpp>

namespace libbitcoin {
namespace database {

class BCD_API stealth_database
{
public:
    typedef std::function<void(memory_ptr)> write_function;

    /// Construct the database.
    stealth_database(const boost::filesystem::path& rows_filename,
        std::shared_ptr<shared_mutex> mutex=nullptr);

    /// Close the database (all threads must first be stopped).
    ~stealth_database();

    /// Initialize a new stealth database.
    bool create();

    /// Call before using the database.
    bool start();

    /// Call to signal a stop of current operations.
    bool stop();

    /// Call to unload the memory map.
    bool close();

    /// Linearly scan all entries, discarding those after from_height.
    chain::stealth_compact::list scan(const binary& filter,
        size_t from_height) const;

    /// Add a stealth row to the database.
    void store(uint32_t prefix, uint32_t height,
        const chain::stealth_compact& row);

    /// Delete all rows after and including from_height (no implemented).
    void unlink(size_t from_height);

    /// Synchronise storage with disk so things are consistent.
    /// Should be done at the end of every block write.
    void sync();

private:
    void write_index();
    array_index read_index(size_t from_height) const;

    // Row entries containing stealth tx data.
    memory_map rows_file_;
    record_manager rows_manager_;
};

} // namespace database
} // namespace libbitcoin

#endif
