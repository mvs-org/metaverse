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

        account_remark_database::account_remark_database(const boost::filesystem::path& lookup_filename,
                                                         const boost::filesystem::path& rows_filename,
                                                         std::shared_ptr<shared_mutex> mutex)
                : account_remark_table(lookup_filename, mutex),
                  account_remark_rows(rows_filename, mutex)
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
            return account_remark_table.create() && account_remark_rows.create();
        }

        // Startup and shutdown.
        // ----------------------------------------------------------------------------

        // Start files and primitives.
        bool account_remark_database::start()
        {
            return account_remark_table.start() && account_remark_rows.start();
        }

        // Stop files.
        bool account_remark_database::stop()
        {
            return account_remark_table.stop() && account_remark_rows.stop();
        }

        // Close files.
        bool account_remark_database::close()
        {
            return account_remark_table.close() && account_remark_rows.close();
        }

        void account_remark_database::sync()
        {
            account_remark_table.sync();
            account_remark_rows.sync();
        }

        void account_remark_database::remove(const std::string& account_name)
        {
            const hash_digest&& hash = get_hash(account_name);
            account_remark_table.remove(hash);
        }

        void account_remark_database::store(const std::string& account_name, const hash_digest& tx_hash, const std::string& remark)
        {
            const hash_digest&& hash = get_hash(account_name);
            file_offset position = max_uint64;
            {
                const auto memory = account_remark_table.get_lookup_map().find(hash);
                if (memory) {
                    auto deserial = make_deserializer_unsafe(REMAP_ADDRESS(memory));
                    position = deserial.read_8_bytes_little_endian();
                }
            }

            auto write_row = [&remark](memory_ptr data)
            {
                auto serial = make_serializer(REMAP_ADDRESS(data));
                serial.write_string(remark);
            };
            const auto value_size = variable_uint_size(remark.size()) + remark.size() + 1; // reserve 1 byte for future extension

            const file_offset new_position = account_remark_rows.get_lookup_map().store(tx_hash, position, write_row, value_size);

            auto write_table = [&new_position](memory_ptr data)
            {
                auto serial = make_serializer(REMAP_ADDRESS(data));
                serial.write_8_bytes_little_endian(new_position);
            };
            if (position != max_uint64) {
                account_remark_table.get_lookup_map().restore(hash, write_table, value_size);
            } else {
                account_remark_table.get_lookup_map().store(hash, write_table, value_size);
            }
        }

        const std::string account_remark_database::get(const std::string& account_name, const hash_digest& tx_hash)
        {
            const hash_digest&& hash = get_hash(account_name);

            const auto memory = account_remark_table.get_lookup_map().find(hash);
            if (memory) {
                auto deserial = make_deserializer_unsafe(REMAP_ADDRESS(memory));
                const file_offset position = deserial.read_8_bytes_little_endian();
                {
                    const auto memory = account_remark_rows.get_lookup_map().find(tx_hash, position);
                    auto deserial = make_deserializer_unsafe(REMAP_ADDRESS(memory));
                    return deserial.read_string();
                }
            }
            return "";
        }

        const std::map<hash_digest, std::string> account_remark_database::get_all(const std::string& account_name)
        {
            std::map<hash_digest, std::string> ret;

            const hash_digest&& hash = get_hash(account_name);

            const auto memory = account_remark_table.get_lookup_map().find(hash);
            if (memory) {
                auto deserial = make_deserializer_unsafe(REMAP_ADDRESS(memory));
                const file_offset position = deserial.read_8_bytes_little_endian();
                {
                    const auto vec_memory_ptr = account_remark_rows.get_lookup_map().finds(position);
                    for (const auto &memory: vec_memory_ptr) {
                        auto deserial = make_deserializer_unsafe(REMAP_ADDRESS(memory));
                        const hash_digest&& tx_hash = deserial.read_hash();
                        deserial.read_bytes<sizeof(file_offset)>(); // discard next
                        ret[tx_hash] = deserial.read_string();
                    }

                }
            }

            return ret;
        };
    }
}