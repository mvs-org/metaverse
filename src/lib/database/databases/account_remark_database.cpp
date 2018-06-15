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

#include <metaverse/database/databases/account_remark_database.hpp>


namespace libbitcoin {
    namespace database {
        using namespace boost::filesystem;

        BC_CONSTEXPR size_t number_buckets = 65500; //65535 means invalid index
        BC_CONSTEXPR size_t header_size = slab_hash_table_header_size(number_buckets);
        BC_CONSTEXPR size_t initial_map_file_size = header_size + minimum_slabs_size;

        account_remark_database::account_remark_database(const path& map_filename,
                                                         std::shared_ptr<shared_mutex> mutex)
                : lookup_file_(map_filename, mutex),
                  lookup_header_(lookup_file_, number_buckets),
                  lookup_manager_(lookup_file_, header_size),
                  lookup_map_(lookup_header_, lookup_manager_)
        {}

        account_remark_database::~account_remark_database()
        {
            close();
        }

        // Create.
        // ----------------------------------------------------------------------------

        // Initialize files and start.
        bool account_remark_database::create()
        {
            // Resize and create require a started file.
            if (!lookup_file_.start())
                return false;

            // This will throw if insufficient disk space.
            lookup_file_.resize(initial_map_file_size);

            if (!lookup_header_.create() ||
                !lookup_manager_.create())
                return false;

            // Should not call start after create, already started.
            return
                    lookup_header_.start() &&
                    lookup_manager_.start();
        }

        // Startup and shutdown.
        // ----------------------------------------------------------------------------

        // Start files and primitives.
        bool account_remark_database::start()
        {
            return
                    lookup_file_.start() &&
                    lookup_header_.start() &&
                    lookup_manager_.start();
        }

        // Stop files.
        bool account_remark_database::stop()
        {
            return lookup_file_.stop();
        }

        // Close files.
        bool account_remark_database::close()
        {
            return lookup_file_.close();
        }

        void account_remark_database::sync()
        {
            lookup_manager_.sync();
        }

        const uint16_t account_remark_database::allocate_index()
        {
            return lookup_map_.allocate_index();
        }

        void account_remark_database::free_index(const uint16_t& index )
        {
            lookup_map_.free_index(index);
        }

        void account_remark_database::store(const uint16_t& account_index, const hash_digest& tx_hash, const std::string& remark)
        {

            const auto && key = get_key(account_index, tx_hash);

            auto write = [&remark](memory_ptr data)
            {
                auto serial = make_serializer(REMAP_ADDRESS(data));
                serial.write_string(remark);
            };

            const auto value_size = variable_uint_size(remark.size()) + remark.size() + 1; // reserve 1 byte for future extension

            lookup_map_.store(key, write, value_size);
        }

        const std::string account_remark_database::get(const uint16_t& account_index, const hash_digest& tx_hash) const
        {
            const auto && key = get_key(account_index, tx_hash);

            const auto memory = lookup_map_.find(key);
            if (memory) {
                auto deserial = make_deserializer_unsafe(REMAP_ADDRESS(memory));
                return deserial.read_string();
            } else {
                return "";
            }
        }

        const std::map<hash_digest, std::string> account_remark_database::get_all(const uint16_t& account_index) const
        {
            std::map<hash_digest, std::string> ret;

            std::vector<memory_ptr> data;
            lookup_map_.finds_by_index(account_index, data);
            for (const auto &memory: data) {
                auto deserial = make_deserializer_unsafe(REMAP_ADDRESS(memory));
                const hash_digest&& tx_hash = deserial.read_hash();
                deserial.read_bytes<sizeof(file_offset)>(); // discard next
                ret[tx_hash] = deserial.read_string();
            }

            return ret;
        };
    }
}